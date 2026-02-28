module; // 1. Global Module Fragment

#include <string>
#include <string_view>
#include <cstdint>
#include <iostream>
#include <stdexcept>

export module TaskModule; // 2. Declare the Module Name

export using TaskId = uint32_t;

export enum class TaskPriority {
    Low,
    Normal,
    High,
    Critical
};

export class TaskValidationException : public std::runtime_error
{
public:
    explicit TaskValidationException(const std::string &msg)
        : std::runtime_error(msg) {}
};

export class ITask
{
public:
    virtual ~ITask() = default;

    [[nodiscard]] virtual TaskId GetId() const = 0;
    [[nodiscard]] virtual std::string_view GetTitle() const = 0;
    [[nodiscard]] virtual bool IsDone() const = 0;

    virtual void Complete() = 0;
    virtual void PrintDetails() const = 0;
};

// BaseTask remains abstract because it doesn't implement PrintDetails()
export class BaseTask : public ITask
{
protected:
    TaskId id;
    std::string title;
    bool isCompleted;

public:
    // NG/TS MAGIC: 'bool status = false' makes this parameter OPTIONAL!
    // No need for multiple constructors anymore.
    BaseTask(TaskId taskId, std::string_view taskTitle, bool status = false)
        : id(taskId), title(taskTitle), isCompleted(status)
    {
        if (title.empty())
        {
            throw TaskValidationException("Title cannot be empty, bro!");
        }
        std::cout << "[DEBUG] Base Constructor initialized Task ID: " << id << "\n";
    }

    [[nodiscard]] TaskId GetId() const override { return id; }
    [[nodiscard]] std::string_view GetTitle() const override { return title; }
    [[nodiscard]] bool IsDone() const override { return isCompleted; }

    void Complete() override { isCompleted = true; }
};

export class FeatureTask : public BaseTask
{
private:
    std::string epicName;

public:
    // Notice how we pass 'false' to the BaseTask (or leave it empty if we want the default)
    FeatureTask(TaskId taskId, std::string_view taskTitle, std::string_view epic)
        : BaseTask(taskId, taskTitle), epicName(epic) {}

    ~FeatureTask() override
    {
        std::cout << "[MEMORY] FeatureTask '" << title << "' destroyed safely!\n";
    }

    void PrintDetails() const override
    {
        std::cout << "[FEATURE] " << title << " (Epic: " << epicName << ")\n";
    }
};

export class BugFixTask : public BaseTask
{
private:
    TaskPriority priority;

public:
    BugFixTask(TaskId taskId, std::string_view taskTitle, TaskPriority bugPriority)
        : BaseTask(taskId, taskTitle), priority(bugPriority) {}

    ~BugFixTask() override
    {
        std::cout << "[MEMORY] BugFixTask '" << title << "' destroyed safely!\n";
    }

    void PrintDetails() const override
    {
        std::string priorityStr = (priority == TaskPriority::Critical) ? "CRITICAL" : "NORMAL";
        std::cout << "[BUGFIX]  " << title << " (Priority: " << priorityStr << ")\n";
    }
};