#pragma once
#include <cmath>
#include <stdexcept>
#include <type_traits>
#include <vector>

template <typename Mydeque, typename ValueType>
class Deque_Iterator_Base {
    using size_type_ = typename Mydeque::size_type;
    static constexpr int block_size_ = Mydeque::block_size_;

  public:
    using value_type = typename Mydeque::value_type;
    using difference_type = typename Mydeque::difference_type;
    using pointer = ValueType*;
    using reference = ValueType&;
    using iterator_category = std::random_access_iterator_tag;

  private:
    typename Mydeque::pointer* map_pointer_ = nullptr;
    difference_type block_current_pos_ = 0;

    pointer object_ptr_ = nullptr;

    pointer calc_ptr() const {
        if (block_current_pos_ != 0) {
            return *map_pointer_;
        }
        return nullptr;
    }

  public:
    Deque_Iterator_Base(typename Mydeque::pointer* map_pointer,
                        difference_type block_current_pos_)
        : map_pointer_(map_pointer),
          block_current_pos_(block_current_pos_),
          object_ptr_(calc_ptr()) {}
    template <typename U = ValueType>
    Deque_Iterator_Base(
        const typename Mydeque::iterator& it,
        std::enable_if_t<std::is_const_v<U>>* /*unused*/ = nullptr)
        : map_pointer_(it.map_pointer_),
          block_current_pos_(it.block_current_pos_),
          object_ptr_(calc_ptr()) {}

    reference operator*() const noexcept {
        return *operator->();
    }
    pointer operator->() const noexcept {
        if (object_ptr_ != nullptr) {
            return object_ptr_ + block_current_pos_;
        }
        return (*map_pointer_) + block_current_pos_;
    }

    Deque_Iterator_Base& operator+=(
        difference_type offset) noexcept {  // REWATCH
        difference_type map_offset;
        bool object_ptr_updated = (0 <= block_current_pos_ + offset) &&
                                  (block_current_pos_ + offset < block_size_);
        if (offset >= -block_current_pos_) {
            map_offset = (block_current_pos_ + offset) / block_size_;
            block_current_pos_ = (block_current_pos_ + offset) % block_size_;
        } else {
            map_offset = -static_cast<difference_type>(std::ceil(
                std::abs(static_cast<double>(block_current_pos_ + offset) /
                         block_size_)));
            block_current_pos_ = (block_size_ + block_current_pos_ -
                                  (std::abs(offset) % block_size_)) %
                                 block_size_;
        }
        map_pointer_ += map_offset;
        if (!object_ptr_updated) {
            object_ptr_ = calc_ptr();
        }
        return *this;
    }
    Deque_Iterator_Base& operator-=(const difference_type offset) noexcept {
        *this += -offset;
        return *this;
    }

    Deque_Iterator_Base& operator++() noexcept {
        *this += 1;
        return *this;
    }
    Deque_Iterator_Base& operator--() noexcept {
        *this -= 1;
        return *this;
    }

    Deque_Iterator_Base operator++(int) noexcept {
        Deque_Iterator_Base temp = *this;
        ++*this;
        return temp;
    }
    Deque_Iterator_Base operator--(int) noexcept {
        Deque_Iterator_Base temp = *this;
        --*this;
        return temp;
    }

    bool operator==(const Deque_Iterator_Base& it) const noexcept {
        return this->map_pointer_ == it.map_pointer_ &&
               this->block_current_pos_ == it.block_current_pos_;
    }
    bool operator!=(const Deque_Iterator_Base& it) const noexcept {
        return !(*this == it);
    }
    bool operator<(const Deque_Iterator_Base& it) const noexcept {
        return this->map_pointer_ < it.map_pointer_ ||
               (this->map_pointer_ == it.map_pointer_ &&
                this->block_current_pos_ < it.block_current_pos_);
    }
    bool operator>(const Deque_Iterator_Base& it) const noexcept {
        return it < *this;
    }
    bool operator<=(const Deque_Iterator_Base& it) const noexcept {
        return !(it < *this);
    }
    bool operator>=(const Deque_Iterator_Base& it) const noexcept {
        return !(*this < it);
    }

    template <typename Mydeque_, typename ValueType_1, typename ValueType_2>
    friend typename Deque_Iterator_Base<Mydeque_, ValueType_1>::difference_type
    operator-(const ::Deque_Iterator_Base<Mydeque_, ValueType_1>& it1,
              const Deque_Iterator_Base<Mydeque_, ValueType_2>& it2);

    friend Deque_Iterator_Base<Mydeque, const ValueType>;
};

template <typename Mydeque, typename ValueType>
Deque_Iterator_Base<Mydeque, ValueType> operator+(
    Deque_Iterator_Base<Mydeque, ValueType> it,
    typename Deque_Iterator_Base<Mydeque, ValueType>::difference_type offset) {
    it += offset;
    return it;
}

template <typename Mydeque, typename ValueType>
Deque_Iterator_Base<Mydeque, ValueType> operator-(
    Deque_Iterator_Base<Mydeque, ValueType> it,
    typename Deque_Iterator_Base<Mydeque, ValueType>::difference_type offset) {
    it -= offset;
    return it;
}

template <typename Mydeque, typename ValueType>
Deque_Iterator_Base<Mydeque, ValueType> operator+(
    typename Deque_Iterator_Base<Mydeque, ValueType>::difference_type offset,
    Deque_Iterator_Base<Mydeque, ValueType> it) {
    it += offset;
    return it;
}

template <typename Mydeque, typename ValueType1, typename ValueType2>
typename Deque_Iterator_Base<Mydeque, ValueType1>::difference_type operator-(
    const Deque_Iterator_Base<Mydeque, ValueType1>& it1,
    const Deque_Iterator_Base<Mydeque, ValueType2>& it2) {
    using It_Type = Deque_Iterator_Base<Mydeque, ValueType1>;
    return (it1.map_pointer_ - it2.map_pointer_) * It_Type::block_size_ -
           it2.block_current_pos_ + it1.block_current_pos_;
}

template <typename T>
class Deque {
  public:
    using value_type = T;
    using size_type = size_t;
    using difference_type = int;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;

    using iterator = Deque_Iterator_Base<Deque<value_type>, value_type>;
    using const_iterator =
        Deque_Iterator_Base<Deque<value_type>, const value_type>;

  private:
    static constexpr int min_map_size_ = 8;
    static constexpr int block_size_ = 64;

    size_type size_ = 0;
    size_type start_ = (min_map_size_ / 2) * block_size_;

  public:
    std::vector<pointer> map_ = std::vector<pointer>(min_map_size_);

    Deque() {}
    Deque(const Deque<T>& other) : Deque() {
        int count = other.size();
        reserve(count);
        std::pair<int, int> pos = get_pos(0);
        while (count > 0) {
            for (pos.second = 0; pos.second < std::min(count, block_size_);
                 ++pos.second) {
                new (map_[pos.first] + pos.second) T(other[size_]);
                ++size_;
            }
            ++pos.first;
            count -= block_size_;
        }
    }
    Deque(int count) : Deque(count, T()) {}
    Deque(int count, const T& value) : Deque() {
        reserve(count);
        std::pair<int, int> pos = get_pos(0);
        while (count > 0) {
            for (pos.second = 0; pos.second < std::min(count, block_size_);
                 ++pos.second) {
                new (map_[pos.first] + pos.second) T(value);
                ++size_;
            }
            ++pos.first;
            count -= block_size_;
        }
    }

    Deque<T>& operator=(Deque<T> other) {
        swap(other);
        return *this;
    }

    ~Deque() {
        clear();
        for (pointer& p : map_) {
            free_block(p);
        }
    }

    reference operator[](int i) {
        std::pair<int, int> pos = get_pos(i);
        return map_[pos.first][pos.second];
    }
    const_reference operator[](int i) const {
        std::pair<int, int> pos = get_pos(i);
        return map_[pos.first][pos.second];
    }

    reference at(size_type i) {
        if (i >= size()) {
            throw std::out_of_range("Index out of range!");
        }
        return (*this)[i];
    }
    const_reference at(size_type i) const {
        if (i >= size()) {
            throw std::out_of_range("Index out of range!");
        }
        return (*this)[i];
    }

    void push_back(const T& value) {
        std::pair<int, int> pos = get_pos(size());
        if (pos.first < static_cast<int>(map_.size())) {
            if (map_[pos.first] != nullptr) {  // first case
                new (map_[pos.first] + pos.second) T(value);
            } else {  // second case
                map_[pos.first] = allocate_block();
                try {
                    new (map_[pos.first] + pos.second) T(value);
                } catch (...) {
                    free_block(map_[pos.first]);
                    throw;
                }
            }
            ++size_;
        } else {  // third case
            std::vector<pointer> new_map{map_.capacity() * 2};
            copy_values(new_map, map_);
            start_ += (new_map.size() - map_.size()) / 2 * block_size_;
            map_.swap(new_map);
            try {
                push_back(value);
            } catch (...) {
                map_.swap(new_map);
                start_ -= (new_map.size() - map_.size()) / 2 * block_size_;
                throw;
            }
        }
    }
    void push_front(const T& value) {
        if (start_ != 0) {
            std::pair<int, int> pos = get_pos(-1);
            if (map_[pos.first] != nullptr) {  // first case
                new (map_[pos.first] + pos.second) T(value);
            } else {  // second case
                map_[pos.first] = allocate_block();
                try {
                    new (map_[pos.first] + pos.second) T(value);
                } catch (...) {
                    free_block(map_[pos.first]);
                    throw;
                }
            }
            ++size_;
            --start_;
        } else {  // third case
            std::vector<pointer> new_map{map_.size() * 2};
            copy_values(new_map, map_);
            start_ = (new_map.size() / 2 - map_.size() / 2) * block_size_;
            map_.swap(new_map);
            try {
                push_front(value);
            } catch (...) {
                map_.swap(new_map);
                start_ = 0;
                throw;
            }
        }
    }

    void pop_back() noexcept {
        std::pair<int, int> pos = get_pos(size() - 1);
        map_[pos.first][pos.second].~T();
        --size_;
    }
    void pop_front() noexcept {
        std::pair<int, int> pos = get_pos(0);
        map_[pos.first][pos.second].~T();
        ++start_;
        --size_;
    }

    void insert(iterator it, const T& value) {
        difference_type offset = it - begin();
        push_back(value);  // "it" is invalidated
        place_in(--end(), begin() + offset);
    }
    void erase(iterator it) {
        std::swap(*it, *(--end()));
        pop_back();
        place_in(rbegin() + (end() - it), rbegin());
    }

    size_type size() const noexcept {
        return size_;
    }

    iterator begin() noexcept {
        std::pair<int, int> pos = get_pos(0);
        return iterator(&map_[pos.first], pos.second);
    }
    const_iterator cbegin() const noexcept {
        std::pair<int, int> pos = get_pos(0);
        return const_iterator(const_cast<pointer*>(&map_[pos.first]),  // NOLINT
                              pos.second);
    }
    const_iterator begin() const noexcept {
        return cbegin();
    }

    iterator end() noexcept {
        if (size() == 0) {
            return begin();
        }
        std::pair<int, int> pos = get_pos(size() - 1);
        if (pos.second == block_size_ - 1) {
            return iterator(&map_[pos.first] + 1, 0);
        }
        return iterator(&map_[pos.first], pos.second + 1);
    }
    const_iterator cend() const noexcept {
        if (size() == 0) {
            return cbegin();
        }
        std::pair<int, int> pos = get_pos(size() - 1);
        if (pos.second == block_size_ - 1) {
            return const_iterator(
                const_cast<pointer*>(&map_[pos.first] + 1),  // NOLINT
                0);
        }
        return const_iterator(const_cast<pointer*>(&map_[pos.first]),  // NOLINT
                              pos.second + 1);
    }
    const_iterator end() const noexcept {
        return cend();
    }

    auto rbegin() noexcept {
        return std::reverse_iterator<iterator>(end());
    }
    auto crbegin() const noexcept {
        return std::reverse_iterator<const_iterator>(end());
    }
    auto rbegin() const noexcept {
        return std::reverse_iterator<const_iterator>(end());
    }

    auto rend() noexcept {
        return std::reverse_iterator<iterator>(begin());
    }
    auto crend() const noexcept {
        return std::reverse_iterator<const_iterator>(begin());
    }
    auto rend() const noexcept {
        return std::reverse_iterator<const_iterator>(begin());
    }

  private:
    std::pair<int, int> get_pos(difference_type pos) const noexcept {
        return {(static_cast<int>(start_) + pos) / block_size_,
                (static_cast<int>(start_) + pos) % block_size_};
    }

    void reserve(int count) {
        int amount = std::ceil(static_cast<double>(count) / block_size_);
        if (amount == 0) {
            return;
        }
        increase(std::max(0, static_cast<int>(std::ceil(std::log2(amount))) -
                                 static_cast<int>(std::log2(map_.size())) + 1));
        for (int i = start_ / block_size_; amount > 0; ++i) {
            if (map_[i] == nullptr) {
                map_[i] = allocate_block();
            }
            --amount;
        }
    }

    void increase(int n) {  // increase capacity to the power of 2 times
        if (n == 0) {
            return;
        }
        std::vector<pointer> new_map{map_.size() * (1 << n)};
        copy_values(new_map, map_);
        start_ += (new_map.size() - map_.size()) / 2 * block_size_;
        map_.swap(new_map);
    }

    static void copy_values(
        std::vector<pointer>& v1,
        const std::vector<pointer>& v2) {  // v2.size() <= v1.size()
        size_type offset = (v1.size() - v2.size()) / 2;
        for (size_type i = 0; i < v2.size(); ++i) {
            v1[offset + i] = v2[i];
        }
    }

    static pointer allocate_block() {
        return reinterpret_cast<pointer>(new char[block_size_ * sizeof(T)]);
    }
    static void free_block(pointer& p) noexcept {
        delete[] reinterpret_cast<char*>(p);
        p = nullptr;
    }

    void clear() {
        for (size_type i = 0; i < size(); ++i) {
            at(i).~T();
        }
        start_ = (map_.size() / 2) * block_size_;
        size_ = 0;
    }

    void swap(Deque& other) {
        std::swap(map_, other.map_);
        std::swap(start_, other.start_);
        std::swap(size_, other.size_);
    }

    template <typename It_Type>
    static void place_in(It_Type movable, It_Type dest) {
        if (movable != dest) {
            std::swap(*movable, *dest);
            return place_in(movable, ++dest);
        }
    }

    template <typename Mydeque, typename ValueType>
    friend class Deque_Iterator_Base;
};
