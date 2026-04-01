# Introduction to Low-Level Design (LLD)

---

## 1. What is Low-Level Design (LLD)?

**Definition:**  
Designing the internal structure (“skeleton”) of an application by identifying:
- Classes / objects  
- Relationships  
- Data flow  
- How DSA solutions fit into the system  

### DSA vs LLD

- **DSA:** Solves isolated problems  
  - Examples: Binary search, Quicksort, Dijkstra’s, Heaps  

- **LLD:**  
  - Decides what objects exist  
  - Defines how they interact  
  - Then applies DSA within this structure  

---

## 2. Illustrative Story: QuickRide

**Scenario:** Build a ride-booking app like Uber/Ola  

---

### Anurag’s DSA-First Approach

**Problem decomposition:**
- City → graph (nodes = intersections, edges = roads)  
- Use Dijkstra’s → shortest path  
- Use min-heap → match riders to closest drivers  

**Gaps:**
- No entities (User, Rider, Location, etc.)  
- No data security (phone masking missing)  
- No integrations (notifications, payments)  
- No scalability planning  

---

### Maurya’s LLD-First Approach

**1. Entity identification:**
- User  
- Rider  
- Location  
- NotificationService  
- PaymentGateway  

**2. Relationships & interactions:**
- User and Rider connected via Location  
- Notification and Payment services integrated  

**3. Non-functional concerns:**
- Data security  
- Scalability  

**4. Apply DSA:**
- Shortest path for routing  
- Heap for driver matching  

---

## 3. Core LLD Principles

### Scalability
- Handle large number of users  
- Easy to expand system  

### Maintainability
- Easy to debug  
- Changes should not break existing features  

### Reusability
- Loosely coupled modules  
- Example: reusable notification system  

---

## 4. What LLD is NOT (vs HLD)

**LLD does NOT include:**
- Tech stack (Java, Spring Boot, etc.)  
- Database choice (SQL / NoSQL)  
- Deployment (AWS, GCP, scaling)  
- Cost optimization  

---

## 5. Summary

- **DSA:** Solves specific problems  
- **LLD:** Defines structure and interactions  
- **HLD:** Defines overall system architecture  

---

## 6. Key Line

> If DSA is the brain, LLD is the skeleton of your application.