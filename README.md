# Banking Management System

## Overview

This project is a Banking Management System developed using C++ and basic Data Structures and Algorithms (DSA).

The system simulates core banking operations such as:

* account creation
* balance management
* withdrawal
* loan management
* EMI payment
* transaction history
* customer service queue

The main objective of this project is to demonstrate the implementation of different data structures and algorithms in a real-world banking scenario.

---

# Features

## Account Management

* Create bank accounts
* Store customer details
* Generate unique account numbers
* Delete accounts
* Display all accounts

## Banking Operations

* Check balance
* Withdraw money
* Transfer money
* Manage account data

## Loan System

* Loan approval system
* EMI calculation
* EMI payment tracking
* Loan eligibility checking
* Remaining amount tracking

## File Handling

* Persistent data storage using JSON
* Automatic loading of account data on startup
* Automatic saving after updates

## DSA Implementations

* Dynamic Arrays using Vector
* Linear Search
* Stack
* Queue
* Sorting Algorithms
* File Organization

---

# Data Structures Used

## Vector (Dynamic Array)

Used to store:

* account data
* loan data

Reason:

* dynamic memory allocation
* efficient traversal
* easy insertion

Example:

```cpp id="gxjlwm"
vector<Account> accounts;
```

---

## Stack

Used for:

* undo transaction functionality
* transaction history tracking

Principle:
LIFO (Last In First Out)

Example:

```cpp id="jlwm4r"
stack<pair<int,double>> undoStack;
```

---

## Queue

Used for:

* customer service requests
* loan processing order

Principle:
FIFO (First In First Out)

Example:

```cpp id="jlwm6t"
queue<pair<int,string>> serviceQueue;
```

---

# Algorithms Used

## Linear Search

Used to find accounts using account number.

Complexity:

O(n)

---

## Sorting

Insertion Sort and STL sort are used for sorting account data.

Complexity:

Insertion Sort:
O(n^2)

STL sort:
O(n\log n)

---

# Loan Logic

The system checks whether the customer is eligible for a loan based on account balance.

Condition:
Customer must maintain at least 10% of the requested loan amount in their account.

The system then:

* calculates EMI
* stores loan details
* tracks remaining payment

---

# Interest Logic

Interest is the extra amount paid by the customer for borrowing money from the bank.

The system calculates monthly EMI using:

* loan amount
* interest rate
* repayment duration

---

# File Handling

The system uses:

```cpp id="jlwm7c"
accounts.json
```

for permanent storage.

Functions:

* save()
* load()

Purpose:

* save account data permanently
* reload data when program starts again

This process is called Persistent Storage.

---

# Time Complexity Summary

| Operation         | Complexity         |
| ----------------- | ------------------ |
| Account Search    | O(n)               |
| Account Insertion | O(1)               |
| Withdrawal        | O(n)               |
| Traversal         | O(n)               |
| Sorting           | O(n²) / O(n log n) |

---

# Technologies Used

* C++
* STL
* File Handling
* JSON Storage
* Data Structures and Algorithms

---

# Conclusion

This project demonstrates how basic DSA concepts can be applied in a real-world banking system.

The project combines:

* account management
* loan processing
* transaction handling
* persistent storage
* DSA optimization

to simulate core banking operations efficiently.
