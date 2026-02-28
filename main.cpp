#include <iostream>

import TaskModule;
import ProjectManagerModule;

int main()
{
    std::cout << "--- Starting Application ---\n\n";

    {
        // We create a scope (block) to demonstrate memory destruction
        ProjectManager manager;

        manager.AddFeature("Implement C++20 Modules", "Architecture Epic");
        manager.AddBugFix("Fix IntelliSense Squiggles", TaskPriority::Critical);
        manager.AddFeature("Write a Custom CLI", "DevEx Epic");

        // Complete a specific task
        if (auto task = manager.FindTaskById(2))
        {
            task.value()->Complete();
            std::cout << "\n[System] Marked ID 2 as completed.\n";
        }

        manager.PrintReport();

        std::cout << "--- Manager is about to go out of scope ---\n";
    } // <-- HERE is where the magic happens! 'manager' is destroyed.

    std::cout << "\n--- Application Finished Successfully ---\n";
    return 0;
}

/* int main()
{
    try
    {
        std::cout << "Step 1: Creating valid task...\n";
        // FIX: We must instantiate a specific class (FeatureTask), not the abstract BaseTask!
        FeatureTask task1(101, "Learn C++ Default Arguments", "C++ Mastery");

        std::cout << "\nStep 2: Trying to create INVALID task (Empty title)...\n";
        // This will explode!
        FeatureTask task2(102, "", "C++ Mastery");
    }
    catch (const TaskValidationException &ex)
    {
        std::cerr << "\n [CATCHED ERROR]: " << ex.what() << std::endl;
    }
    catch (const std::exception &ex)
    {
        std::cerr << "System error: " << ex.what() << std::endl;
    }

    std::cout << "\n--- App continues to run safely ---\n";
    return 0;
} */
