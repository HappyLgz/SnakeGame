#ifndef BOUNDEDQUEUE_H
#define BOUNDEDQUEUE_H
#include <array>
#include <optional>

template<typename T, std::size_t N>
class FixedFifo {
    std::array<T, N> buffer{};
    std::size_t head = 0;   // 下一个写入位置
    std::size_t tail = 0;   // 下一个读取位置
    bool full = false;      // 区分空和满状态

public:
    void push(const T& item) {
        buffer[head] = item;
        if (full) {
            tail = (tail + 1) % N;   // 覆盖时移动读取指针
        }
        head = (head + 1) % N;
        full = (head == tail);
    }

    T pop() {
        // 如果确信不空，才调用此函数
        T item = buffer[tail];
        tail = (tail + 1) % N;
        full = false;
        return item;
    }

    bool empty() const { return (!full && (head == tail)); }
    bool is_full() const { return full; }
    std::size_t capacity() const { return N; }
};

#endif // BOUNDEDQUEUE_H
