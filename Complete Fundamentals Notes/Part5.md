# System Design — Detailed Personal Notes (Chapter 5)

**Topics:** Caching, Redis, invalidation strategies, cache-aside / write-through / write-back, production pitfalls

These notes continue from [Chapter 4 — SQL/NoSQL, Microservices & Load Balancing](Part4.md). Every concept is explained from first principles with real-world analogies, diagrams, and worked examples.

**Previous ←** [Chapter 4: SQL/NoSQL, Microservices & Load Balancing](Part4.md) · **Next →** [Chapter 6: Blob Storage, CDN & Message Brokers](Part6.md)

---

# PART 1: CACHING — The Complete Deep Dive

## Why Caching Exists — Building The Intuition From Scratch

Before diving into Redis commands and code, let's build a completely solid mental model of WHY caching is necessary and what problem it's actually solving.

Every time a user requests data from your application, this is what happens without caching:

```
User's browser
     │
     │  GET /blogs  (HTTP request travels over the internet)
     ▼
Application Server
     │
     │  Runs business logic:
     │  - Parse the request
     │  - Validate authentication token
     │  - Build the SQL/MongoDB query
     │
     │  SELECT * FROM blogs ORDER BY created_at DESC LIMIT 20;
     ▼
Database Server (on disk)
     │
     │  - Read index from disk to find matching rows  (~50ms)
     │  - Read actual row data from disk              (~200ms)
     │  - Send data back over internal network        (~10ms)
     ▼
Application Server (receives raw DB data)
     │
     │  - Deserialize the data
     │  - Apply business logic / formatting
     │  - Serialize to JSON                           (~100ms)
     ▼
User's browser
     
Total round trip: ~360ms just for ONE request.
```

Now imagine 10,000 users hit `/blogs` every minute. That's 10,000 times your database is reading the same rows off disk, your server is running the same query and the same formatting logic. The data hasn't changed. You're doing identical work thousands of times per minute and charging your CPU and database for it every single time.

This is the problem caching solves. You compute the result ONCE, store it somewhere ultra-fast, and serve all 10,000 users from that fast store. The database doesn't even know the other 9,999 requests happened.

**WITHOUT CACHING:**
10,000 requests/minute
→ 10,000 database queries/minute
→ Database at 80% load
→ 360ms response time for everyone

**WITH CACHING:**
10,000 requests/minute
→ 1 database query (first request only, or after cache expires)
→ 9,999 requests served from Redis
→ Database at ~1% load
→ 20ms response time for everyone

That's not a 2x improvement. That's a 9,999x reduction in database load and an 18x improvement in response time. This is why caching is one of the highest-leverage things you can add to any system.

---

## The Mental Model — Cache Hit vs Cache Miss

These two terms are the foundation of all caching logic. Every caching system in existence operates on this exact two-path model.

### Cache Hit — The Happy Path

A cache hit means: "The data the user requested is already in the cache. Serve it directly."

```
CACHE HIT — Step by step:

Step 1: Client sends request to Server
        GET /blogs
        Client ──────────────────────▶ Server

Step 2: Server asks Redis: "Do you have blogData?"
        Server ──────────────────────▶ Redis
        
Step 3: Redis responds: "Yes! Here it is." (20ms)
        Server ◀────────────────────── Redis
        
Step 4: Server sends cached data directly to Client
        Client ◀────────────────────── Server

TOTAL TIME: ~20-60ms
DATABASE: Never touched. Didn't even know a request happened.
```

### Cache Miss — The Slower Path (But Still Correct)

A cache miss means: "The data isn't in the cache. We need to go to the database, then populate the cache so next time is fast."

```
CACHE MISS — Step by step:

Step 1: Client sends request to Server
        Client ──────────────────────▶ Server

Step 2: Server asks Redis: "Do you have blogData?"
        Server ──────────────────────▶ Redis
        
Step 3: Redis responds: "Nope, nothing here." (5ms)
        Server ◀────────────────────── Redis
        ("Not found in cache" message from Redis)

Step 4: Server queries the actual Database
        Server ──────────────────────▶ Database

Step 5: Database returns the data (300-500ms)
        Server ◀────────────────────── Database

Step 6: Server stores data in Redis for future requests
        Server ──────────────────────▶ Redis
        (SET blogData <data> EX 86400)  ← stored with 24hr TTL

Step 7: Server returns data to Client
        Client ◀────────────────────── Server

TOTAL TIME: ~500ms (same as without cache — but only THIS request pays this cost)
NEXT REQUEST: Will be a cache HIT — 20ms
```

The first request after a cache miss (or after cache expiry) always pays the full database cost. Every subsequent request until the cache expires gets the fast path. This is a completely acceptable trade-off.

---

## Cache Invalidation — The Hardest Problem in Caching

There's a famous quote in computer science by Phil Karlton: *"There are only two hard things in Computer Science: cache invalidation and naming things."*

Cache invalidation means: **deciding when cached data is stale and needs to be refreshed.**

Why is this hard? Because your cache is a copy of the database. The moment the database changes, the cache is potentially outdated (stale). You need a strategy for how and when to update or clear the cache.

Let's go through all the real strategies with their trade-offs.

### Strategy 1: TTL (Time to Live) — Expiry-Based Invalidation

The simplest strategy. You store data in the cache with an expiry time. After that time passes, Redis automatically deletes it. The next request will be a cache miss, fetch fresh data from DB, and repopulate the cache.

```
SET blogData <json_data> EX 86400   ← EX = expire, 86400 = 24 hours in seconds

Timeline:
t=0:      Cache populated. TTL = 86400 seconds.
t=1:      User requests /blogs → Cache HIT. Returns cached data. 20ms.
t=3600:   50,000 users have been served from cache.
t=86400:  Redis automatically DELETES blogData. Cache is empty.
t=86401:  Next user requests /blogs → Cache MISS. DB queried. Data refreshed.
t=86402:  All subsequent users → Cache HIT again. 20ms.

And the cycle repeats every 24 hours.
```

**When TTL works perfectly:** For data that changes infrequently and where slight staleness is acceptable. Blog posts, product catalog, trending content, leaderboards — if the data shown is 30 minutes or 2 hours old, the user probably doesn't care or notice.

**When TTL fails:** For data that must be real-time accurate. If your TTL is 24 hours and a product goes out of stock 1 hour after caching, 23 hours of users see "In Stock" when it isn't. For inventory or pricing, this is unacceptable.

**Choosing the right TTL is a genuine design decision:**

Very short TTL (seconds to minutes):
  Good for: Live sports scores, stock prices, real-time dashboards
  Trade-off: Cache provides less benefit (expires too often)
  
Medium TTL (minutes to hours):
  Good for: News feeds, product listings, user profiles
  Trade-off: Slight staleness is usually fine for these
  
Long TTL (hours to days):
  Good for: Static content, configuration, rarely-changing master data
  Trade-off: Stale data risk increases; need active invalidation for updates

**No TTL (infinite):**
- Good for: Truly static data (country list, currency codes)
- Trade-off: Must invalidate manually when data changes

### Strategy 2: Active Invalidation on Write — Delete or Update Cache When Data Changes

Instead of waiting for TTL to expire, you actively delete or update the cache the moment the underlying data changes.

**SCENARIO:** New blog post published.

**WITHOUT ACTIVE INVALIDATION:**
  - New blog saved to database
  - Cache still has old blog list ← STALE
  - Users see old list until TTL expires (could be 23 hours!)
  - "Where's my new blog? I just published it!" — author is confused

**WITH ACTIVE INVALIDATION:**
  - New blog saved to database
  - Server immediately: DEL blogData (deletes from Redis)
  - Next user to request /blogs → Cache MISS → fetches fresh data with new blog
  - Cache repopulated with new data
  - All subsequent users see the new blog immediately

Code flow:
// When publishing a new blog post:
await database.insert(newBlog);         // Save to DB
await redisClient.del('blogData');      // Invalidate cache
// Done. Next request will re-populate the cache automatically.

**Pattern: Cache-Aside (most common)**

This is the most widely used caching pattern. The application code manages when to read from cache and when to invalidate.

READ path (cache-aside):
1. Check cache
2. If HIT: return cached data
3. If MISS: fetch from DB, store in cache, return data

WRITE path (cache-aside):
1. Write to database
2. Invalidate (delete) the cache entry
3. Next read will re-populate the cache

Simple, but requires both DB write AND cache delete to succeed.
If cache delete fails, cache is stale until TTL expires.

**Pattern: Write-Through Cache**

Write to the cache AND the database simultaneously on every write operation. Cache is always up-to-date.

```
The Codeforces Example from the notes:

User submits a competitive programming solution.
Server does TWO writes simultaneously:
  Write 1: UPDATE leaderboard in PostgreSQL database
  Write 2: SET leaderboard:live <new_ranking_data> in Redis

**When 100,000 users refresh the leaderboard:**
→ All served from Redis
→ Redis always has the most current data
→ Database write happened, Redis write happened at the same time

This is write-through caching.

ADVANTAGE: Cache is always perfectly in sync with database.
           No staleness ever.

DISADVANTAGE: Every single write operation hits BOTH the database 
              AND Redis. Write latency is higher.
              Also, if you write data that nobody ever reads,
              you're wasting Redis memory caching it for no reason.
              (Write-through caches data whether it's popular or not)
```

**Pattern: Write-Back (Write-Behind) Cache**

The riskiest but fastest pattern. Write to cache FIRST, database LATER (asynchronously).

```
User action: Update profile photo.

Write-back flow:
1. Server writes to Redis immediately. 
   Responds to user: "Updated!" (very fast — just Redis write)
2. In the background, a separate job periodically:
   Reads all pending writes from Redis
   Writes them to the actual database

ADVANTAGE: Extremely fast writes. Database isn't in the critical path.

MASSIVE RISK: If Redis crashes BEFORE the background job 
              flushes to the database:
              ALL pending writes are LOST.
              User's profile update? Gone.
              
Use only when: You can tolerate data loss for this particular data,
               AND write performance is critical.
               Example: View counts, click counts — losing 
               a few views is acceptable vs. perfect write performance.
```

---

## Types of Caches — Where Caching Happens

Caching isn't just Redis on a server. It happens at multiple layers of the entire request journey.

### 1. Client-Side Cache (Browser Cache)

When you visit a website, your browser automatically caches certain resources locally on your device. The next time you visit, those resources load from your local disk instead of being downloaded again.

First visit to www.flipkart.com:
Browser downloads:
  - flipkart-logo.png     (150 KB)   → Cache-Control: max-age=31536000 (1 year)
  - main.bundle.js        (800 KB)   → Cache-Control: max-age=31536000 (1 year)
  - styles.css            (200 KB)   → Cache-Control: max-age=31536000 (1 year)
  - homepage HTML         (50 KB)    → Cache-Control: no-cache (always fresh)
  
Total downloaded: ~1200 KB. Page loads in 3 seconds.

Second visit (next day):
Browser checks cache:
  - flipkart-logo.png  → In cache, not expired (1 year TTL). Load locally. 0ms.
  - main.bundle.js     → In cache, not expired. Load locally. 0ms.
  - styles.css         → In cache, not expired. Load locally. 0ms.
  - homepage HTML      → no-cache means always check server for fresh version.
  
Total downloaded: ~50 KB (just the HTML). Page loads in 0.3 seconds.
10x faster on the second visit.

The server controls browser caching behavior through HTTP response headers:

Cache-Control: max-age=31536000    ← Cache for 1 year (static assets with content hash in filename)
Cache-Control: no-cache            ← Always validate with server before using cached version
Cache-Control: no-store            ← Never cache (sensitive data like banking pages)
Cache-Control: private             ← Cache only in browser, not in intermediate proxies
Cache-Control: public              ← Can be cached by browser AND CDNs

- ETag: "abc123"                     ← A fingerprint of the content.
- Browser sends: "If-None-Match: abc123"
- Server: "Content hasn't changed → 304 Not Modified"
- Browser: Uses cached version. No download needed.

**Why static assets use content hashes in their filenames:**

When you deploy new code, Webpack/Vite generates filenames like:
  main.a3f8b2c.js   (the hash changes every time code changes)

Browser has cached: main.a3f8b2c.js with 1-year TTL.
You deploy new code: main.e9d1f5a.js  (different hash = different filename)

Browser never had main.e9d1f5a.js in cache → downloads fresh version.
Old main.a3f8b2c.js stays in cache but is never referenced again.

This gives you:
  - Infinite cache TTL (safe because filename changes on every deploy)
  - Instant cache busting on new deployments (new filename = cache miss)
This technique is called "cache busting."

### 2. Server-Side Cache (Redis, Memcached)

This is what most people mean when they say "caching" in a backend context. Data is stored in a fast in-memory store on the server side. We'll go deep on Redis in the next section.

### 3. CDN Cache (Content Delivery Network)

A CDN is a globally distributed network of servers (called "edge servers" or "Points of Presence" — PoPs) that cache your content close to your users geographically.

**WITHOUT CDN:**
User in Chennai wants to watch a video hosted on servers in Mumbai.

- Journey: Chennai user → Internet → Mumbai Data Center → back to Chennai
- Physical distance: ~1300 km one way
- Speed of light in fiber: ~200,000 km/s
- Round trip time just from physics: ~13ms minimum
- With network overhead: ~60-100ms per request

**For a 100 MB video:**
- Entire file travels 1300 km from Mumbai
- If Mumbai is under load (lots of users): slowdowns, buffering

**WITH CDN:**
User in Chennai wants to watch the same video.

**First user from Chennai to watch this video:**
1. CDN edge server in Chennai: "I don't have this video cached."
2. Edge server fetches from origin (Mumbai): one time trip.
3. Edge server caches the video locally in Chennai.

**ALL subsequent users in Chennai:**
- CDN edge server in Chennai: "I have this video! Serving locally."
- Distance: a few km within Chennai.
- Latency: < 5ms
- Speed: Full bandwidth, no Mumbai involved at all.

GLOBAL SCALE:
Flipkart serves product images from 50+ CDN PoPs worldwide.
A user in New York gets images from a New York edge server.
A user in London gets images from a London edge server.
The origin server in Mumbai is only contacted when a PoP 
doesn't have the content yet.

**What gets cached at CDN vs what doesn't:**

**CACHE AT CDN (static content that doesn't change per user):**
- Images (product photos, logos, banners)
- Videos
- CSS, JavaScript bundles
- Fonts
- Static HTML pages
- PDF files, downloadable content

**DO NOT CACHE AT CDN (dynamic content specific to a user):**
- User's dashboard ("Hello, Rahul! Your orders: ...")
- Shopping cart contents
- Authentication tokens or session data
- Payment pages
- Any page showing personalized data

### 4. Application-Level Cache

This is caching within the application code itself, often in-process (in the same memory as the application).

```javascript
// Simple in-process cache using a Map
const cache = new Map();

async function getExpensiveComputationResult(input) {
    // Check if result is already cached in memory
    if (cache.has(input)) {
        console.log('Returning from in-process cache');
        return cache.get(input);
    }
    
    // Not cached — do the expensive work
    const result = await someExpensiveCalculation(input);  // takes 2 seconds
    
    // Cache the result in memory
    cache.set(input, result);
    
    return result;
}
```

**Difference between application cache and Redis:**

**Application-Level Cache (in-process Map/Dict):**
+ Absolute fastest — no network call, just memory lookup
- Lost when server restarts
- NOT shared across multiple server instances
+ Good for: Config values, compiled regex patterns

**Redis (out-of-process):**
+ Shared across all server instances
+ Survives server restarts (with persistence)
+ Has TTL support, rich data types, pub/sub
- Requires a network call (still very fast: 1-5ms)
+ Good for: Session data, shared computed results

---

## Redis — Complete Deep Dive

### What Redis Actually Is

Redis stands for **RE**mote **DI**ctionary **S**erver. The word "dictionary" is the key insight — it's fundamentally a dictionary (key-value store) that lives in RAM.

**Why RAM makes Redis so fast:**

Storage medium comparison:

HDD (Hard Disk Drive):
  Random access: ~1 ms (mechanical arm must physically move)

SSD (Solid State Drive):
  Random access: ~0.1ms

**RAM (Random Access Memory):**
- Latency: ~60 nanoseconds (0.00006ms)
- Typical Redis lookup latency: ~0.1-1ms (includes network)

Redis can handle 100,000 to 1,000,000 operations/sec on one instance.

**Why not use Redis for EVERYTHING if it's so fast?**

**RAM:**
- Speed: Blazing fast
- Cost: Expensive
- Volatility: Loses data on power loss by default

**SSD/Disk:**
- Speed: Slower
- Cost: Much cheaper
- Persistence: Survives power cuts

So we cache only hot data in Redis.
80/20 rule: 80% of requests often hit 20% of data.

### Redis Key Naming Conventions

Redis keys are strings, but conventions matter.

Use colon-separated namespaces:

- user:1
- user:1:email
- user:1:session
- product:456
- product:456:stock
- leaderboard:game:xyz
- rate:ip:192.168.1.1
- cache:blogs:page:1
- otp:phone:9876543210

Pattern scanning:
SCAN 0 MATCH "user:*"

---

## Redis Data Types — In Depth

### Data Type 1: Strings

```
SET user:1 "Shivam"
GET user:1
MGET user:1 user:2

SET otp:9876543210 "847291" EX 300
TTL otp:9876543210

SET user:1 "Shivam" NX

SET page_views:blog:123 0
INCR page_views:blog:123
INCRBY page_views:blog:123 10
```

Real-world string use cases:
- Cache full API responses
- Session tokens
- Rate limiting counters
- OTP storage
- Feature flags

### Data Type 2: Lists

LPUSH user Amit
RPUSH user Abhay
LLEN user
LPOP user
RPOP user
LRANGE user 0 -1
BRPOP job_queue 30

Use cases:
- FIFO job queues
- LIFO stacks
- Recently viewed lists
- Background workers

### Data Type 3: Hashes

- HSET user:1 name "Shivam" age 21 city "Delhi" email "shivam@gmail.com"
- HGET user:1 name
- HGETALL user:1
- HMGET user:1 name email
- HDEL user:1 city

Use cases:
- User profiles
- Session objects
- Product metadata
- Config blobs

### Data Type 4: Sets

SADD active_users "user_1" "user_2" "user_3"
SMEMBERS active_users
SISMEMBER active_users "user_2"
SCARD active_users

SINTER team_a team_b
SUNION team_a team_b
SDIFF team_a team_b

Use cases:
- Unique visitors
- Likes membership checks
- Mutual friends
- Tag collections

### Data Type 5: Sorted Sets (ZSets)

ZADD leaderboard 9850 "player_rahul"
ZADD leaderboard 9920 "player_shivam"
ZRANGE leaderboard 0 -1 WITHSCORES
ZREVRANGE leaderboard 0 2 WITHSCORES
ZRANK leaderboard "player_rahul"
ZREVRANK leaderboard "player_rahul"
ZINCRBY leaderboard 500 "player_ankit"
ZSCORE leaderboard "player_shivam"

Use cases:
- Leaderboards
- Priority queues
- Sliding-window rate limiting
- Trending topics
- Delayed job scheduling

---

## The Blog Cache — Complete Code Walkthrough

```javascript
const Redis = require('ioredis');
const redisClient = new Redis();

async function checkCache(req, res, next) {
    const cachedData = await redisClient.get('blogData');
    
    if (cachedData) {
        console.log('Data retrieved from cache');
        res.json(JSON.parse(cachedData));
    } else {
        next();
    }
}

app.get('/blog', checkCache, async (req, res) => {
    try {
        const response = await axios.get('https://api.example.com/blog');
        const blogData = response.data;
        
        await redisClient.set('blogData', JSON.stringify(blogData), 'EX', 86400);
        res.json(blogData);
        
    } catch (error) {
        res.status(500).json({ error: 'Internal server error' });
    }
});
```

Add invalidation on write:

```javascript
app.post('/blog', async (req, res) => {
    try {
        const newBlog = req.body;
        await database.blogs.insert(newBlog);
        await redisClient.del('blogData');
        res.json({ success: true, blog: newBlog });
    } catch (error) {
        res.status(500).json({ error: 'Internal server error' });
    }
});
```

Write-through leaderboard example:

```javascript
async function submitSolution(userId, problemId, solution) {
    const newScore = await judgeAndScore(solution);
    
    await database.query(
        'UPDATE scores SET score = ? WHERE user_id = ? AND problem_id = ?',
        [newScore, userId, problemId]
    );
    
    await redisClient.zadd('leaderboard:live', newScore, `user:${userId}`);
}
```

---

## Cache Hit Rate — The Metric That Tells You If Caching Is Working

Cache hit rate = (cache hits) / (total requests) × 100%

Example:
Total requests: 100,000
Cache hits:      95,000
Cache misses:     5,000
Hit rate: 95%

Interpretation:
- `<50%`: caching strategy likely weak
- `50-80%`: decent, improvable
- `80-95%`: good
- `>95%`: excellent

Check Redis stats:

```
redis-cli INFO stats | grep keyspace_hits
redis-cli INFO stats | grep keyspace_misses
```

---

## Common Caching Problems and How to Handle Them

### Problem 1: Cache Stampede (Thundering Herd)

A hot key expires and thousands of requests hit DB at once.

Mitigations:
- Locking on cache refresh (`SET NX EX`)
- Probabilistic early refresh
- TTL jitter (randomized expiry)

### Problem 2: Cache Penetration

Attackers request non-existent IDs repeatedly, bypassing cache.

Mitigations:
- Cache null/not-found responses briefly
- Bloom filters for fast non-existent checks

### Problem 3: Hot Key Problem

One viral key receives huge read traffic.

Mitigations:
- In-process local cache on app servers
- Key replication (fan-out reads across replicated keys)

---

## Redis in Production — What You Actually Need to Know

### Redis Persistence

RDB snapshots:
  save 900 1
  save 300 10
  save 60 10000

AOF:
  Append every write command, replay on restart.

Common production setup:
  Use both RDB + AOF.

### Redis Cluster

```
Redis Cluster uses 16384 hash slots.

HASH_SLOT = CRC16(key) % 16384

- Node 1: slots 0-5460
- Node 2: slots 5461-10922
- Node 3: slots 10923-16383

Clients route keys directly by slot.
Replicas provide failover.
```

---

## Summary — The Mental Model for When to Cache What

Use this checklist:

1. Is data frequently accessed by many users?
2. Is it expensive to compute/fetch?
3. Is slight staleness acceptable?
4. Is data shared or user-specific?
5. How often does it change?

Priority order for caching:
1. Home/landing shared data
2. Product listings/search results
3. Session data
4. Leaderboards/trending
5. Config/feature flags
6. User-specific data
7. Single-record lookups

That's caching from first principles through production patterns: hit/miss, invalidation, Redis structures, failure modes, and operational considerations.

---

## Quick Reference — Chapter 5

| Concept | One-line summary |
|---------|------------------|
| Cache hit | Data in cache — fast path, DB untouched |
| Cache miss | Fetch from DB, populate cache, then respond |
| TTL | Auto-expire stale data — simple, may show old data briefly |
| Cache-aside | App reads cache first; on write, invalidate cache |
| Write-through | Write to DB and cache together — always fresh |
| Write-back | Write cache first, DB later — fast but risky |
| Redis | In-memory key-value store — sub-ms reads, rich types |
| Stampede | Many misses at once — use locks, jitter, early refresh |
| Hot key | One key overloaded — local cache or key replicas |

**Previous ←** [Chapter 4: SQL/NoSQL, Microservices & Load Balancing](Part4.md) · **Next →** [Chapter 6: Blob Storage, CDN & Message Brokers](Part6.md)
