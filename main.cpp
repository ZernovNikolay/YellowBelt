#include <iostream>
#include <algorithm>
#include <string>
#include <map>
#include <tuple>

// Перечислимый тип для статуса задачи
enum class TaskStatus {
    NEW,          // новая
    IN_PROGRESS,  // в разработке
    TESTING,      // на тестировании
    DONE          // завершена
};

// Объявляем тип-синоним для map<TaskStatus, int>,
// позволяющего хранить количество задач каждого статуса
using TasksInfo = std::map<TaskStatus, int>;

TasksInfo operator+(TasksInfo& lhs, TasksInfo& rhs) {

    TasksInfo RT;

    for (int fooInt = 0; fooInt != static_cast<int>(TaskStatus::DONE)+1; fooInt++)
    {
        TaskStatus foo = static_cast<TaskStatus>(fooInt);
        RT[foo] = lhs[foo] + rhs[foo];

    }

    return RT;
};

class TeamTasks {
public:
    // Получить статистику по статусам задач конкретного разработчика
    const TasksInfo& GetPersonTasksInfo(const std::string& person) const {
        return full.at(person);
    };

    // Добавить новую задачу (в статусе NEW) для конкретного разработчитка
    void AddNewTask(const std::string& person) {
        full[person][TaskStatus::NEW]++;
    };

    // Обновить статусы по данному количеству задач конкретного разработчика,
    // подробности см. ниже
    std::tuple<TasksInfo, TasksInfo> PerformPersonTasks(
        const std::string& person, int task_count) {

        TasksInfo upgraded_tasks;
        TasksInfo updated_tasks;
        TasksInfo new_map = full.at(person);

        TasksInfo t = full.at(person);

        for (auto a : t) {

            if (task_count != 0) {

                switch (a.first) {

                    case (TaskStatus::NEW): {

                        int task_old = a.second;
                        if (task_old < task_count) {
                            task_count -= task_old;
                            upgraded_tasks[TaskStatus::IN_PROGRESS] = task_old;
                            updated_tasks[TaskStatus::NEW] = 0;
                        }
                        else {
                            
                            upgraded_tasks[TaskStatus::IN_PROGRESS] = task_count;
                            updated_tasks[TaskStatus::NEW] = task_old - task_count;
                            task_count = 0;

                        }

                        break;

                        //new_map[TaskStatus::NEW] = updated_tasks[TaskStatus::NEW];

                    }


                    case (TaskStatus::IN_PROGRESS): {

                        int task_old = a.second;
                        if (task_old < task_count) {
                            task_count -= task_old;
                            upgraded_tasks[TaskStatus::TESTING] = task_old;
                            updated_tasks[TaskStatus::IN_PROGRESS] = 0;
                        }
                        else {
                            
                            upgraded_tasks[TaskStatus::TESTING] = task_count;
                            updated_tasks[TaskStatus::IN_PROGRESS] = task_old - task_count;
                            task_count = 0;

                        }

                        //new_map[TaskStatus::IN_PROGRESS] = updated_tasks[TaskStatus::IN_PROGRESS] + upgraded_tasks[TaskStatus::IN_PROGRESS];


                        break;
                    }


                    case(TaskStatus::TESTING): {

                        int task_old = a.second;
                        if (task_old < task_count) {
                            task_count -= task_old;
                            upgraded_tasks[TaskStatus::DONE] = task_old;
                            updated_tasks[TaskStatus::TESTING] = 0;
                        }
                        else {
                            
                            upgraded_tasks[TaskStatus::DONE] = task_count;
                            updated_tasks[TaskStatus::TESTING] = task_old - task_count;
                            task_count = 0;

                        }

                        //new_map[TaskStatus::TESTING] = updated_tasks[TaskStatus::TESTING] + upgraded_tasks[TaskStatus::TESTING];


                        break;

                    }


                    case(TaskStatus::DONE): {

                    }

                    default:
                        break;

                }

            }
            else {
                break;
            }

        }

        auto kt = upgraded_tasks + updated_tasks;

        full.at(person) = upgraded_tasks + updated_tasks;

        return std::make_tuple(upgraded_tasks, updated_tasks);

    };

private:
    std::map<std::string, TasksInfo> full;
};

void PrintTasksInfo(TasksInfo tasks_info) {
    std::cout << tasks_info[TaskStatus::NEW] << " new tasks" <<
        ", " << tasks_info[TaskStatus::IN_PROGRESS] << " tasks in progress" <<
        ", " << tasks_info[TaskStatus::TESTING] << " tasks are being tested" <<
        ", " << tasks_info[TaskStatus::DONE] << " tasks are done" << std::endl;
}

int main() {
    TeamTasks tasks;
    tasks.AddNewTask("Ilia");
    for (int i = 0; i < 8; ++i) {
        tasks.AddNewTask("Ivan");
    }
    std::cout << "Ilia's tasks: ";
    PrintTasksInfo(tasks.GetPersonTasksInfo("Ilia"));
    std::cout << "Ivan's tasks: ";
    PrintTasksInfo(tasks.GetPersonTasksInfo("Ivan"));

    TasksInfo updated_tasks, untouched_tasks;

    tie(updated_tasks, untouched_tasks) =
        tasks.PerformPersonTasks("Ivan", 2);
    std::cout << "Updated Ivan's tasks: ";
    PrintTasksInfo(updated_tasks);
    std::cout << "Untouched Ivan's tasks: ";
    PrintTasksInfo(untouched_tasks);

    tie(updated_tasks, untouched_tasks) =
        tasks.PerformPersonTasks("Ivan", 2);
    std::cout << "Updated Ivan's tasks: ";
    PrintTasksInfo(updated_tasks);
    std::cout << "Untouched Ivan's tasks: ";
    PrintTasksInfo(untouched_tasks);

    return 0;
}