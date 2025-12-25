# Bank Management System (C Language)

## 📌 Project Overview
This is a console-based Bank Management System developed in C that simulates real-world banking operations.  
The system supports secure user authentication, file-based data persistence, transaction logging, daily transfer limits, and loan management for both customers and employees.

---

## ⚙️ Features

### 👤 User Management
- Sign Up & Sign In with secure 5-digit PIN
- Customer and Employee roles
- Employee verification using admin code

### 💰 Banking Operations
- Deposit and Withdraw funds
- Balance inquiry
- Money transfer between users
- Daily transfer limit enforcement
- Transaction history logging
- **Account summary with recent transaction history**

### 🏦 Loan Management
- Loan eligibility based on:
  - Employee years of service
  - Customer account balance
- Interest calculation
- Monthly installment calculation

### 📂 File Handling
- Persistent storage using text files
- `users.txt` stores all account data
- `transactions.txt` stores complete transaction history

### 🛡 Data Validation
- Input validation for:
  - Numeric values
  - CNIC (13 digits)
  - Date of Birth format (DD/MM/YYYY)
  - PIN security
- Prevents invalid or unsafe operations

---

## 🛠 Technologies Used
- C Programming Language
- File Handling
- Structures
- Time & Date Handling
- Menu-driven Console Interface

---

## 📁 Project Structure

---

## ▶️ How to Run the Project

1. Clone the repository:
