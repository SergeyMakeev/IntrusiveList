#include "intrusive_list/intrusive_list.hpp"
#include <iostream>
#include <string>

// Example class that can be stored in an intrusive list
struct Task {
    std::string name;
    int priority;
    
    // The intrusive list node - this is what makes the object "intrusive"
    intrusive::list_node link;
    
    Task(const std::string& n, int p) : name(n), priority(p) {}
    
    // Optional: custom destructor logic
    ~Task() {
        std::cout << "Task '" << name << "' destroyed\n";
    }
};

// Another example with multiple list nodes (object can be in multiple lists)
struct Employee {
    std::string name;
    int id;
    
    intrusive::list_node dept_link;    // Can be in department list
    intrusive::list_node project_link; // Can be in project list simultaneously
    
    Employee(const std::string& n, int i) : name(n), id(i) {}
};

int main() {
    std::cout << "=== Intrusive List Example ===\n\n";
    
    // Create a list of tasks
    intrusive::intrusive_list<&Task::link> task_list;
    
    // Create some task objects
    Task task1("Write documentation", 1);
    Task task2("Fix bug #123", 3);
    Task task3("Review code", 2);
    
    std::cout << "1. Adding tasks to the list:\n";
    task_list.push_back(task1);
    task_list.push_back(task2);
    task_list.push_front(task3); // Higher priority, goes to front
    
    // Iterate through the list
    std::cout << "Tasks in order:\n";
    for (const auto& task : task_list) {
        std::cout << "  - " << task.name << " (priority: " << task.priority << ")\n";
    }
    
    std::cout << "\n2. Using iterators:\n";
    auto it = task_list.begin();
    std::cout << "First task: " << it->name << "\n";
    ++it;
    std::cout << "Second task: " << it->name << "\n";
    
    std::cout << "\n3. Accessing front and back:\n";
    std::cout << "Front: " << task_list.front().name << "\n";
    std::cout << "Back: " << task_list.back().name << "\n";
    
    std::cout << "\n4. Removing a specific task:\n";
    task_list.erase(task2); // Remove by object reference
    std::cout << "After removing 'Fix bug #123':\n";
    for (const auto& task : task_list) {
        std::cout << "  - " << task.name << "\n";
    }
    
    std::cout << "\n5. Checking if objects can be inserted:\n";
    Task new_task("New task", 1);
    std::cout << "Can insert new_task? " << 
        (intrusive::intrusive_list<&Task::link>::can_insert(new_task) ? "Yes" : "No") << "\n";
    std::cout << "Can insert task1? " << 
        (intrusive::intrusive_list<&Task::link>::can_insert(task1) ? "Yes" : "No") << "\n";
    
    std::cout << "\n6. Multiple lists example:\n";
    
    // Create employees
    Employee emp1("Alice", 101);
    Employee emp2("Bob", 102);
    Employee emp3("Carol", 103);
    
    // Create different lists
    intrusive::intrusive_list<&Employee::dept_link> engineering_dept;
    intrusive::intrusive_list<&Employee::project_link> project_alpha;
    
    // Same employees can be in both lists simultaneously
    engineering_dept.push_back(emp1);
    engineering_dept.push_back(emp2);
    engineering_dept.push_back(emp3);
    
    project_alpha.push_back(emp1); // Alice works on project alpha
    project_alpha.push_back(emp3); // Carol works on project alpha
    
    std::cout << "Engineering department:\n";
    for (const auto& emp : engineering_dept) {
        std::cout << "  - " << emp.name << " (ID: " << emp.id << ")\n";
    }
    
    std::cout << "Project Alpha team:\n";
    for (const auto& emp : project_alpha) {
        std::cout << "  - " << emp.name << " (ID: " << emp.id << ")\n";
    }
    
    std::cout << "\n7. Move semantics example:\n";
    
    // Create a list and move it
    intrusive::intrusive_list<&Task::link> list1;
    Task temp_task("Temporary task", 5);
    list1.push_back(temp_task);
    
    // Move the list
    auto list2 = std::move(list1);
    std::cout << "After move - list1 is empty: " << (list1.empty() ? "Yes" : "No") << "\n";
    std::cout << "list2 contains: " << list2.front().name << "\n";
    
    std::cout << "\n8. Automatic cleanup:\n";
    std::cout << "Objects will be automatically unlinked when destroyed...\n";
    
    return 0;
}