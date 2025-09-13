#ifndef CHAINHASH_H
#define CHAINHASH_H

#include <vector>
#include <functional>
#include <stdexcept>

using namespace std;

const int maxColision = 3;
const float maxFillFactor = 0.8;

template<typename TK, typename TV>
struct ChainHashNode {
    TK key;
    TV value;
    ChainHashNode* next;

    ChainHashNode(const TK& k, const TV& v, ChainHashNode* n = nullptr)
        : key(k), value(v), next(n) {}
};

template<typename TK, typename TV>
class ChainHashListIterator {
public:
    typedef ChainHashNode<TK, TV> Node;

    ChainHashListIterator(Node* node = nullptr) : current(node) {}

    Node& operator*() const {
        return *current;
    }

    Node* operator->() const {
        return current;
    }

    ChainHashListIterator& operator++() {
        if (current) current = current->next;
        return *this;
    }

    ChainHashListIterator operator++(int) {
        ChainHashListIterator tmp(*this);
        ++(*this);
        return tmp;
    }

    bool operator==(const ChainHashListIterator& other) const {
        return this->current == other.current;
    }

    bool operator!=(const ChainHashListIterator& other) const {
        return this->current != other.current;
    }

private:
    Node* current;
};

template<typename TK, typename TV>
class ChainHash
{
private:
    typedef ChainHashNode<TK, TV> Node;
    typedef ChainHashListIterator<TK, TV> Iterator;

    Node** array;  // array de punteros a Node
    int nsize; // total de elementos <key:value> insertados
    int capacity; // tamanio del array
    int *bucket_sizes; // guarda la cantidad de elementos en cada bucket
    int usedBuckets; // cantidad de buckets ocupados (con al menos un elemento)

public:
    ChainHash(int initialCapacity = 10){
        if (initialCapacity <= 0) initialCapacity = 10;
        this->capacity = initialCapacity;
        this->array = new Node*[capacity]();
        this->bucket_sizes = new int[capacity]();
        this->nsize = 0;
        this->usedBuckets = 0;
    }

    TV get(TK key){
        size_t hashcode = getHashCode(key);
        size_t index = hashcode % capacity;

        Node* current = this->array[index];
        while(current != nullptr){
            if(current->key == key) return current->value;
            current = current->next;
        }
        throw std::out_of_range("Key no encontrado");
    }

    int size(){ return this->nsize; }

    int bucket_count(){ return this->capacity; }

    int bucket_size(int index) {
        if(index < 0 || index >= this->capacity) throw std::out_of_range("Indice de bucket invalido");
        return this->bucket_sizes[index];
    }

    void set(TK key, TV value){
        size_t hashcode = getHashCode(key);
        size_t index = hashcode % capacity;

        Node* head = array[index];
        Node* current = head;
        while(current != nullptr){
            if(current->key == key){
                current->value = value;
                return;
            }
            current = current->next;
        }

        Node* newNode = new Node(key, value, head);
        array[index] = newNode;
        if (bucket_sizes[index] == 0) {
            usedBuckets++;
        }
        bucket_sizes[index]++;
        nsize++;

        if (bucket_sizes[index] > maxColision || fillFactor() > maxFillFactor) {
            rehashing();
        }
    }

    bool remove(TK key){
        size_t hashcode = getHashCode(key);
        size_t index = hashcode % capacity;

        Node* current = array[index];
        Node* prev = nullptr;
        while(current != nullptr){
            if(current->key == key){
                if(prev == nullptr){
                    array[index] = current->next;
                } else {
                    prev->next = current->next;
                }
                delete current;
                nsize--;
                bucket_sizes[index]--;
                if(bucket_sizes[index] == 0){
                    usedBuckets--;
                }
                return true;
            }
            prev = current;
            current = current->next;
        }
        return false;
    }

    bool contains(TK key){
        size_t hashcode = getHashCode(key);
        size_t index = hashcode % capacity;

        Node* current = array[index];
        while(current != nullptr){
            if(current->key == key) return true;
            current = current->next;
        }
        return false;
    }

    Iterator begin(int index) {
        if(index < 0 || index >= capacity) throw std::out_of_range("Indice de bucket invalido");
        return Iterator(this->array[index]);
    };
    Iterator end(int index) {
        (void)index;
        return Iterator(nullptr);
    };

private:
    double fillFactor(){
        return (double)this->usedBuckets / (double)this->capacity;
    }

    size_t getHashCode(TK key){
        std::hash<TK> ptr_hash;
        return ptr_hash(key);
    }

    void rehashing(){
        int oldCap = this->capacity;
        int newCap = oldCap * 2 + 1;
        if (newCap <= oldCap) newCap = oldCap + 1; // guard
        Node** newArray = new Node*[newCap]();
        int* new_bucket_sizes = new int[newCap]();
        int newUsedBuckets = 0;

        for(int i = 0; i < oldCap; ++i){
            Node* node = array[i];
            while(node != nullptr){
                Node* nextNode = node->next;
                size_t h = getHashCode(node->key);
                size_t idx = h % newCap;
                node->next = newArray[idx];
                newArray[idx] = node;
                if (new_bucket_sizes[idx] == 0) newUsedBuckets++;
                new_bucket_sizes[idx]++;

                node = nextNode;
            }
        }

        delete [] this->array;
        delete [] this->bucket_sizes;

        this->array = newArray;
        this->bucket_sizes = new_bucket_sizes;
        this->capacity = newCap;
        this->usedBuckets = newUsedBuckets;
    }

public:
    ~ChainHash(){
        if(this->array){
            for(int i = 0; i < this->capacity; ++i){
                Node* current = this->array[i];
                while(current != nullptr){
                    Node* next = current->next;
                    delete current;
                    current = next;
                }
            }
            delete [] this->array;
            this->array = nullptr;
        }
        if(this->bucket_sizes){
            delete [] this->bucket_sizes;
            this->bucket_sizes = nullptr;
        }
    }
};

#endif // CHAINHASH_H
