module;

#include <vector>
#include <memory>
#include <optional>
#include <string_view>
#include <iostream>
#include <algorithm> // For modern algorithms
#include <ranges>    // C++20 Ranges (Like LINQ!)

export module ProjectManagerModule;

import TaskModule;

export class ProjectManager
{
private:
    // This is the heart of C++ memory management.
    // A vector of UNIQUE POINTERS pointing to the INTERFACE.
    // When the vector is destroyed, it calls 'delete' on every pointer automatically.
    std::vector<std::unique_ptr<ITask>> tasks;
    TaskId nextId = 1;

public:
    [[nodiscard]] std::optional<ITask *> FindTaskById(TaskId id)
    {
        // C++20 Ranges approach (Like LINQ's .FirstOrDefault)
        auto it = std::ranges::find_if(tasks, [id](const std::unique_ptr<ITask> &task)
                                       { return task->GetId() == id; });

        if (it != tasks.end())
        {
            return it->get(); // .get() returns the raw observation pointer
        }
        return std::nullopt;
    }

    // Factory method for Features
    void AddFeature(std::string_view title, std::string_view epic)
    {
        // std::make_unique allocates memory on the Heap and immediately wraps it safely
        tasks.push_back(std::make_unique<FeatureTask>(nextId++, title, epic));
        std::cout << "[Manager] Added Feature: " << title << "\n";
    }

    // Factory method for Bugs
    void AddBugFix(std::string_view title, TaskPriority priority)
    {
        tasks.push_back(std::make_unique<BugFixTask>(nextId++, title, priority));
        std::cout << "[Manager] Added BugFix: " << title << "\n";
    }

    void PrintReport() const
    {
        std::cout << "\n================ PROJECT REPORT ================\n";
        for (const auto &task : tasks)
        {
            std::cout << "[" << (task->IsDone() ? "X" : " ") << "] ID: " << task->GetId() << " | ";
            // This is POLYMORPHISM in action!
            // It automatically calls either FeatureTask::PrintDetails or BugFixTask::PrintDetails
            task->PrintDetails();
        }
        std::cout << "================================================\n\n";
    }
};