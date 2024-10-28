#include <memory>
#include <mutex>

template <typename T>
class threadsafe_list {
    struct Node {
        std::shared_ptr<T> data;
        std::unique_ptr<Node> next;
        std::mutex mutex;
        Node():next(){}
        explicit Node(const T& value):data(std::make_shared<T>(value)){}
    };
public:
    Node head;
    threadsafe_list()=default;
    ~threadsafe_list()=default;
    threadsafe_list(const threadsafe_list&) = delete;
    threadsafe_list& operator=(const threadsafe_list&) = delete;

    void push_front(const T& value) {
        std::lock_guard<std::mutex> lock(head.mutex);
        std::unique_ptr<T> new_node = std::make_unique<T>(value);
        new_node->next = std::move(head.next);
        head.next = std::move(new_node);
    }
    template <typename Function>
    void for_each(Function f) {
        const Node* curr=&head;
        std::unique_lock<std::mutex> lock(head.mutex);
        while(const Node* next=curr->next.get()) {
            std::unique_lock<std::mutex> lk(next->mutex);
            lock.unlock();
            f(*next->data);
            curr=next;
            lock=std::move(lk);
        }
    }

    template <typename Predicate>
    std::shared_ptr<T> find_first_of(Predicate p) {
        const Node* curr=&head;
        std::unique_lock<std::mutex> lock(head.mutex);
        while(const Node* next=curr->next.get()) {
            std::unique_lock<std::mutex> lk(next->mutex);
            lock.unlock();
            if(p(*next->data)) {
                return curr->next;
            }
            curr=next;
            lock=std::move(lk);
        }
        return std::shared_ptr<Node>();
    }

    template <typename Predicate>
    void remove_if(Predicate) {
        const Node* curr=&head;
        std::unique_lock<std::mutex> lock(head.mutex);
        while(const Node* next=curr->next.get()) {
            std::unique_lock<std::mutex> lk(next->mutex);
            if(p(*next->data)) {
                std::unique_ptr<Node> tmp=std::move(curr->next);
                curr->next=next->next;
                lk.unlock();
            }else {
                lock.unlock();
                curr=next;
                lock=std::move(lk);
            }

    }
};
