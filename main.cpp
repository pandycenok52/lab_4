#include <iostream>
#include <map>
#include <vector>
#include <memory>
#include <cassert>
#include <limits>

// Класс MyAllocator для управления памятью
template <typename T, size_t BlockSize = 10>
class MyAllocator {
public:
    // Определение типов, связанных с аллокатором
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    // Структура rebind для поддержки аллокации объектов другого типа
    template <typename U>
    struct rebind {
        using other = MyAllocator<U, BlockSize>;
    };

    // Конструктор по умолчанию
    MyAllocator() noexcept = default;

    // Конструктор копирования для других аллокаторов
    template <typename U>
    MyAllocator(const MyAllocator<U, BlockSize>&) noexcept {}

    // Метод для выделения памяти
    pointer allocate(size_type n) {
        // Если запрашивается больше блоков, чем размер блока, используем стандартный оператор new
        if (n > BlockSize) {
            return static_cast<pointer>(::operator new(n * sizeof(T)));
        }
        // Если нет свободных блоков, расширяем пул памяти
        if (free_blocks.empty()) {
            expand();
        }
        // Берем последний свободный блок из вектора free_blocks
        pointer result = free_blocks.back();
        free_blocks.pop_back();
        return result;
    }

    // Метод для освобождения памяти
    void deallocate(pointer p, size_type n) noexcept {
        // Если освобождается больше блоков, чем размер блока, используем стандартный оператор delete
        if (n > BlockSize) {
            ::operator delete(p);
        } else {
            // Добавляем указатель в список свободных блоков
            free_blocks.push_back(p);
        }
    }

    // Метод для конструирования объекта на выделенной памяти
    template <typename U, typename... Args>
    void construct(U* p, Args&&... args) {
        new (p) U(std::forward<Args>(args)...);
    }

    // Метод для разрушения объекта
    template <typename U>
    void destroy(U* p) {
        p->~U();
    }

private:
    // Метод для расширения пула свободной памяти
    void expand() {
        for (size_t i = 0; i < BlockSize; ++i) {
            free_blocks.push_back(static_cast<pointer>(::operator new(sizeof(T))));
        }
    }

    std::vector<pointer> free_blocks; // Вектор для хранения свободных блоков памяти
};

// Операторы сравнения для аллокаторов
template <typename T, typename U, size_t BlockSize>
bool operator==(const MyAllocator<T, BlockSize>&, const MyAllocator<U, BlockSize>&) noexcept {
    return true; // Всегда возвращает true для совместимых аллокаторов
}

template <typename T, typename U, size_t BlockSize>
bool operator!=(const MyAllocator<T, BlockSize>&, const MyAllocator<U, BlockSize>&) noexcept {
    return false; // Всегда возвращает false для совместимых аллокаторов
}

// Шаблонный класс контейнера MyContainer с использованием пользовательского аллокатора
template <typename T, typename Allocator = std::allocator<T>>
class MyContainer {
public:
    using value_type = T;
    using allocator_type = Allocator;
    using pointer = typename std::allocator_traits<Allocator>::pointer;

    // Конструктор контейнера с заданным аллокатором
    MyContainer(const allocator_type& alloc = allocator_type()) : alloc_(alloc), size_(0) {}

    // Метод для добавления элемента в контейнер
    void push_back(const T& value) {
        pointer ptr = std::allocator_traits<Allocator>::allocate(alloc_, 1); // Выделяем память для одного элемента
        std::allocator_traits<Allocator>::construct(alloc_, ptr, value);     // Конструируем элемент на выделенной памяти
        elements_.push_back(ptr);                                            // Сохраняем указатель на элемент в векторе элементов
        ++size_;                                                             // Увеличиваем размер контейнера
    }

    // Метод для вывода элементов контейнера на экран
    void print() const {
        for (const auto& ptr : elements_) {
            std::cout << *ptr << " "; // Разыменовываем указатель и выводим значение элемента
        }
        std::cout << std::endl;       // Переход на новую строку после вывода всех элементов
    }

    size_t size() const { return size_; }  // Возвращает текущий размер контейнера
    bool empty() const { return size_ == 0; }  // Проверяет, пуст ли контейнер

    // Деструктор контейнера для освобождения ресурсов
    ~MyContainer() {
        for (auto ptr : elements_) {
            std::allocator_traits<Allocator>::destroy(alloc_, ptr);          // Разрушаем элемент
            std::allocator_traits<Allocator>::deallocate(alloc_, ptr, 1);     // Освобождаем память
        }
    }

private:
    allocator_type alloc_;       // Аллокатор для управления памятью контейнера
    std::vector<pointer> elements_; // Вектор указателей на элементы контейнера
    size_t size_;                // Текущий размер контейнера
};

// Рекурсивная функция для вычисления факториала числа n
int factorial(int n) {
    if (n <= 1) return 1;         // Базовый случай: факториал 0 и 1 равен 1
    return n * factorial(n - 1);  // Рекурсивный вызов для вычисления факториала n-1
}

int main() {
    // 1. Создание экземпляра std::map<int, int>;
    std::map<int, int> map1;

    // 2. Заполнение 10 элементами, где ключ – это число от 0 до 9, а значение – факториал ключа;
    for (int i = 0; i < 10; ++i) {
        map1[i] = factorial(i);
    }

    // 3. Создание экземпляра std::map<int, int> с новым аллокатором, ограниченным 10 элементами;
    std::map<int, int, std::less<int>, MyAllocator<std::pair<const int, int>, 10>> map2;

    // 4. Заполнение 10 элементами, где ключ – это число от 0 до 9, а значение – факториал ключа;
    for (int i = 0; i < 10; ++i) {
        map2[i] = factorial(i);
    }

    // 5. Вывод на экран всех значений (ключ и значение разделены пробелом), хранящихся в контейнере;
    std::cout << "map1:" << std::endl;
    for (const auto& [key, value] : map1) {
        std::cout << "Key: " << key << ", Value: " << value << std::endl;
    }

    std::cout << "map2:" << std::endl;
    for (const auto& [key, value] : map2) {
        std::cout << "Key: " << key << ", Value: " << value << std::endl;
    }

    // 6. Создание экземпляра своего контейнера для хранения значений типа int;
    MyContainer<int> container1;

    // 7. Заполнение 10 элементами от 0 до 9;
    for (int i = 0; i < 10; ++i) {
        container1.push_back(i);
    }

    // 8. Создание экземпляра своего контейнера для хранения значений типа int с новым аллокатором, ограниченным 10 элементами;
    MyContainer<int, MyAllocator<int, 10>> container2;

    // 9. Заполнение 10 элементами от 0 до 9;
    for (int i = 0; i < 10; ++i) {
        container2.push_back(i);
    }

    // 10. Вывод на экран всех значений, хранящихся в контейнере;
    std::cout << "container1: ";
    container1.print();

    std::cout << "container2: ";
    container2.print();

    return 0;
}