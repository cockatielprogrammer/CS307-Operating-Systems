#include <iostream>
#include <list>
#include <mutex>
#include <thread>


using namespace std;

class HeapManager {
private:
    struct Node {
        int ID;
        int SIZE;
        int INDEX;

        Node(int id, int size, int index) : ID(id), SIZE(size), INDEX(index) {}
    };

    list<Node> heapList;
    mutable mutex heapMutex;

public:
    int initHeap(int size) {
        unique_lock<mutex> lock(heapMutex);

        heapList.clear();
        heapList.emplace_back(-1, size, 0);

        cout << "Memory initialized" << endl;
        print();
        return 1;
    }

    int myMalloc(int ID, int size) {
        unique_lock<mutex> lock(heapMutex);

        for (auto it = heapList.begin(); it != heapList.end(); ++it) {
            if (it->ID == -1 && it->SIZE >= size) {
                if (it->SIZE == size) {
                    it->ID = ID;
                    print();
                    return it->INDEX;
                } else {
                    Node allocated(ID, size, it->INDEX);
                    Node freeSpace(-1, it->SIZE - size, it->INDEX + size);

                    *it = allocated;
                    heapList.insert(next(it), freeSpace);
                    cout << "Allocated for thread " << ID << endl;
                    print();

                    return allocated.INDEX;
                }
            }
        }

        cout << "Can not allocate, requested size " << size << " for thread " << ID << " is bigger than remaining size" << endl;
        print();
       
        return -1;
    }

    int myFree(int ID, int index) {
        unique_lock<mutex> lock(heapMutex);

        for (auto it = heapList.begin(); it != heapList.end(); ++it) {
            if (it->ID == ID && it->INDEX == index) {
                it->ID = -1;

                if (it != heapList.begin() && prev(it)->ID == -1) {
                    it = heapList.erase(prev(it));
                }

                auto nextIt = next(it);
                if (nextIt != heapList.end() && nextIt->ID == -1) {
                    it->SIZE += nextIt->SIZE;
                    heapList.erase(nextIt);
                }

                cout << "Freed for thread " << ID << endl;
                print();
               
                return 1;
            }
        }

        cout << "Thread " << ID << " does not have allocated memory at index " << index << endl;
        print();
      
        return -1;
    }

    void print() {
        auto it = heapList.begin();
        if (it != heapList.end()) {
            cout << "[" << it->ID << "][" << it->SIZE << "][" << it->INDEX << "]";
            ++it;
        }

        while (it != heapList.end()) {
            cout << "---[" << it->ID << "][" << it->SIZE << "][" << it->INDEX << "]";
            ++it;
        }

        cout << endl;
    }
};


