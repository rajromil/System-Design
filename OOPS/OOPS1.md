# OOPS: Abstraction & Encapsulation

---

## 1. Why Did We Move Beyond Procedural Programming?

### 1.1 Early Languages

#### 1. Machine Language (Binary)
- Direct CPU instructions in 0s & 1s  
- Drawbacks:
  - Very error-prone  
  - Hard to write and maintain  
  - No abstraction  

#### 2. Assembly Language
- Uses mnemonics (e.g., MOV A, 61h)  
- Still hardware-dependent  
- Not scalable for large systems  

---

### 1.2 Procedural Programming

**Features:**
- Functions  
- Control structures (if, loops)  
- Code blocks  

**Advantages:**
- Better readability than assembly  
- Modular for small programs  

**Limitations:**
- Poor real-world modeling  
- No proper data security  
- Limited scalability and reuse  

---

## 2. Object-Oriented Programming (OOP)

**Idea:** Model programs using real-world entities as objects  

**Benefits:**
- Maps well to real-world systems  
- Better data security  
- Code reuse (inheritance, interfaces)  
- Scalable and modular  

---

## 3. Basic Concepts

- **Object:** Real-world entity with data + behavior  
- **Class:** Blueprint of object  
- **Instance:** Actual object created from class  

---

## 4. Abstraction

**Definition:**  
Hides implementation details and exposes only essential functionality  

---

### Examples

#### Car
- You use: start, accelerate, brake  
- You don’t see: engine internals  

#### TV / Laptop
- You press buttons  
- You don’t see internal processing  

---

### In Programming

- Using `if`, `for`, `while` instead of machine-level instructions  
- Compiler handles low-level details  

---

### Code Example (Abstract Class)

```cpp
class Car {
public:
    virtual void startEngine() = 0;
    virtual void shiftGear(int newGear) = 0;
    virtual void accelerate() = 0;
    virtual void brake() = 0;
    virtual ~Car() {}
};