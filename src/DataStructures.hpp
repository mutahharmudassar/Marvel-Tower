#pragma once

template<typename T>
struct ListNode {
    T data;
    ListNode* next;
    ListNode* prev;
    ListNode(const T& value) : data(value), next(nullptr), prev(nullptr) {}
};

template<typename T>
class LinkedList {
private:
    ListNode<T>* head;
    ListNode<T>* tail;
    int size;
public:
    LinkedList() : head(nullptr), tail(nullptr), size(0) {}
    ~LinkedList() { clear(); }
    
    void pushBack(const T& value) {
        ListNode<T>* newNode = new ListNode<T>(value);
        if (tail == nullptr) {
            head = tail = newNode;
        } else {
            tail->next = newNode;
            newNode->prev = tail;
            tail = newNode;
        }
        size++;
    }
    
    void remove(ListNode<T>* node) {
        if (node == nullptr) return;
        if (node->prev) node->prev->next = node->next;
        else head = node->next;
        if (node->next) node->next->prev = node->prev;
        else tail = node->prev;
        delete node;
        size--;
    }
    
    void clear() {
        while (head) {
            ListNode<T>* temp = head;
            head = head->next;
            delete temp;
        }
        tail = nullptr;
        size = 0;
    }
    
    ListNode<T>* getHead() { return head; }
    int getSize() const { return size; }
    bool isEmpty() const { return size == 0; }
};

template<typename T>
class Queue {
private:
    struct QueueNode {
        T data;
        QueueNode* next;
        QueueNode(const T& value) : data(value), next(nullptr) {}
    };
    QueueNode* front;
    QueueNode* rear;
    int size;
public:
    Queue() : front(nullptr), rear(nullptr), size(0) {}
    ~Queue() {
        while (front) {
            QueueNode* temp = front;
            front = front->next;
            delete temp;
        }
    }
    
    void enqueue(const T& value) {
        QueueNode* newNode = new QueueNode(value);
        if (rear == nullptr) {
            front = rear = newNode;
        } else {
            rear->next = newNode;
            rear = newNode;
        }
        size++;
    }
    
    bool dequeue(T& value) {
        if (front == nullptr) return false;
        value = front->data;
        QueueNode* temp = front;
        front = front->next;
        if (front == nullptr) rear = nullptr;
        delete temp;
        size--;
        return true;
    }
    
    int getSize() const { return size; }
    bool isEmpty() const { return size == 0; }
};

class CombatLog {
private:
    static const int MAX_LOGS = 10;
    char logs[MAX_LOGS][64];
    int head;
    int count;
public:
    CombatLog() : head(0), count(0) {
        for (int i = 0; i < MAX_LOGS; i++) logs[i][0] = '\0';
    }
    
    void add(const char* message) {
        int i = 0;
        while (message[i] != '\0' && i < 63) {
            logs[head][i] = message[i];
            i++;
        }
        logs[head][i] = '\0';
        head = (head + 1) % MAX_LOGS;
        if (count < MAX_LOGS) count++;
    }
    
    void clear() {
        head = 0;
        count = 0;
    }
    
    int getCount() const { return count; }
    
    const char* getMessage(int index) const {
        if (index < 0 || index >= count) return "";
        int start = (head - count + MAX_LOGS) % MAX_LOGS;
        int idx = (start + index) % MAX_LOGS;
        return logs[idx];
    }
};