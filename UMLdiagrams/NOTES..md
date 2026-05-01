# UML Diagrams - Clear Notes (with mini diagrams)

## 1) UML in simple words
- **UML (Unified Modeling Language)** is a standard way to draw software design.
- Main use:
  - **Static view** -> structure (`Class Diagram`)
  - **Dynamic view** -> behavior over time (`Sequence Diagram`)

---

## 2) Class Diagram Essentials
Class diagram shows:
- classes
- attributes (data)
- methods (functions)
- relationships between classes

### Standard class box
```text
+----------------------+
|        Car           |
+----------------------+
| -brand   : String    |
| -engineCC: int       |
+----------------------+
| +startEngine(): void |
| +brake()      : void |
+----------------------+
```

### Access symbols
- `+` public
- `-` private
- `#` protected

### Common class types
- **Abstract class**: cannot be directly instantiated.
- **Concrete class**: normal class you can create object of.
- **Generic class**: class with type parameter, like `Box<T>`.

---

## 3) Relationship arrows and meanings

### A) Association (basic link)
- Meaning: objects are connected / know each other.
- Symbol: **solid line**.

```text
Customer ---------- Order
```

- Can be:
  - uni-directional: `A ----> B`
  - bi-directional: `A <----> B` (conceptually both sides navigate)

### B) Aggregation (weak has-a)
- Meaning: whole-part, but part can exist independently.
- Symbol: **hollow diamond** at owner side.

```text
Department ◇-------- Teacher
```

If department is removed, teacher can still exist elsewhere.

### C) Composition (strong has-a)
- Meaning: strong ownership; part lifecycle depends on whole.
- Symbol: **filled diamond** at owner side.

```text
House ◆------------- Room
```

If house is destroyed, its rooms are also destroyed in model lifecycle.

### D) Inheritance / Generalization (is-a)
- Meaning: child gets properties/behaviors from parent.
- Symbol: **hollow triangle pointing to parent**.

```text
Dog --------------|> Animal
Car --------------|> Vehicle
```

### E) Dependency (uses temporarily)
- Meaning: one class uses another class for a short task (parameter/local object).
- Symbol: **dashed arrow**.

```text
ReportService - - - - - -> Printer
```

---

## 4) Multiplicity (cardinality)
Multiplicity tells "how many objects" at each end:
- `1` exactly one
- `0..1` optional one
- `*` many
- `1..*` one or many

Diagram examples:
```text
Customer 1 -------- * Order
Order    1 -------- 1 Payment
User     1 -------- 0..1 Profile
```

---

## 5) Quick compare: aggregation vs composition

```text
Aggregation: School ◇---- Student
Composition: House  ◆---- Room
```

- In aggregation, part can survive independently.
- In composition, part is strongly owned by whole.

Use this check:
- "Can part exist without whole?"  
  - Yes -> aggregation  
  - No -> composition

---

## 6) Sequence Diagram Essentials
Sequence diagram shows message flow between participants over time (top -> bottom).

### Key elements
- **Participant**: actor/object (e.g., `User`, `ATM`)
- **Lifeline**: vertical dashed line
- **Activation**: thin rectangle for active execution
- **Message**: arrows between participants

Mini layout:
```text
User          ATM         Account
 |             |            |
 |----req----->|            |
 |             |---check--->|
 |             |<--result---|
 |<--reply-----|            |
```

---

## 7) Message arrow types (important)

### 1. Synchronous call
- Caller waits for response.
- Drawn as solid line arrow.
```text
A ------> B
```

### 2. Asynchronous call
- Caller does not wait; continues work.
- Drawn as solid line with open-style arrow (tool dependent style).
```text
A ----->> B
```

### 3. Return message
- Response from callee to caller.
- Drawn as dashed arrow.
```text
B - - - - -> A
```

### 4. Create message
- One object creates another object.
```text
A ------> B (lifeline of B starts here)
```

### 5. Destroy message
- Object lifecycle ends (`X` at end of lifeline).
```text
A ------> B
            X
```

---

## 8) Combined fragments (control blocks)
- `alt` -> if/else choice
- `opt` -> optional block (if only)
- `loop` -> repeat while/for condition true

Example idea:
```text
alt [balance >= amount]
   ATM -> CashDispenser : dispense()
else [balance < amount]
   ATM -> User : "Insufficient funds"
```

---

## 9) ATM sequence example (clean flow)
Participants: `User`, `ATM`, `Transaction`, `Account`, `CashDispenser`

```text
User        ATM        Transaction      Account      CashDispenser
 |           |               |             |               |
 |--withdraw(amount)-------> |             |               |
 |           |--create txn-> |             |               |
 |           |               |--checkAmt-> |               |
 |           |               |<-valid?---- |               |
 |           |<--status------|             |               |
 |           |--dispense------------------------------->   |
 |           |<--------------------------cash/status----   |
 |<--success/fail--------|             |               |
```

---

## 10) Common mistakes in exams/interviews
- Triangle direction wrong in inheritance (must point to parent).
- Using composition when relationship is actually aggregation.
- Missing multiplicity in associations.
- Writing sequence messages without return/alt/loop when logic clearly has conditions.
- Mixing class diagram semantics (static) with runtime order semantics (sequence).

---

## 11) 30-second revision map
- **is-a** -> `Inheritance` -> `------|>`
- **has-a strong** -> `Composition` -> `◆----`
- **has-a weak** -> `Aggregation` -> `◇----`
- **linked** -> `Association` -> `----`
- **temporarily uses** -> `Dependency` -> `- - ->`
- **time-order interaction** -> `Sequence Diagram`
