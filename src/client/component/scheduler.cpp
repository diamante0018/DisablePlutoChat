#include <std_include.hpp>

#include "../loader/component_loader.hpp"

#include <utils/concurrency.hpp>
#include <utils/hook.hpp>
#include <utils/thread.hpp>

#include "scheduler.hpp"

namespace scheduler
{
  namespace
  {
    struct task
    {
      std::function<bool()> handler{};
      std::chrono::milliseconds interval{};
      std::chrono::high_resolution_clock::time_point last_call{};
    };

    using task_list = std::vector<task>;

    class task_pipeline
    {
     public:
      void add(task&& task)
      {
        new_callbacks_.access(
            [&task, this](task_list& tasks)
            {
              tasks.emplace_back(std::move(task));
            });
      }

      void clear()
      {
        callbacks_.access(
            [&](task_list& tasks)
            {
              this->merge_callbacks();
              tasks.clear();
            });
      }

      void execute()
      {
        callbacks_.access(
            [&](task_list& tasks)
            {
              this->merge_callbacks();

              for (auto i = tasks.begin(); i != tasks.end();)
              {
                const auto now = std::chrono::high_resolution_clock::now();
                const auto diff = now - i->last_call;

                if (diff < i->interval)
                {
                  ++i;
                  continue;
                }

                i->last_call = now;

                const auto res = i->handler();
                if (res == cond_end)
                {
                  i = tasks.erase(i);
                }
                else
                {
                  ++i;
                }
              }
            });
      }

     private:
      utils::concurrency::container<task_list> new_callbacks_;
      utils::concurrency::container<task_list, std::recursive_mutex> callbacks_;

      void merge_callbacks()
      {
        callbacks_.access(
            [&](task_list& tasks)
            {
              new_callbacks_.access(
                  [&](task_list& new_tasks)
                  {
                    tasks.insert(tasks.end(),
                                 std::move_iterator<task_list::iterator>(
                                     new_tasks.begin()),
                                 std::move_iterator<task_list::iterator>(
                                     new_tasks.end()));
                    new_tasks = {};
                  });
            });
      }
    };

    volatile bool kill = false;
    std::thread thread;
    task_pipeline pipelines[pipeline::count];

    void execute(const pipeline type)
    {
      assert(type >= 0 && type < pipeline::count);
      pipelines[type].execute();
    }

    void server_frame_stub()
    {
      reinterpret_cast<void (*)()>(0x50C1E0)();
      execute(pipeline::server);
    }
  } // namespace

  void clear_tasks(const pipeline type)
  {
    return pipelines[type].clear();
  }

  void schedule(const std::function<bool()>& callback, const pipeline type,
                const std::chrono::milliseconds delay)
  {
    assert(type >= 0 && type < pipeline::count);

    task task;
    task.handler = callback;
    task.interval = delay;
    task.last_call = std::chrono::high_resolution_clock::now();

    pipelines[type].add(std::move(task));
  }

  void loop(const std::function<void()>& callback, const pipeline type,
            const std::chrono::milliseconds delay)
  {
    schedule(
        [callback]()
        {
          callback();
          return cond_continue;
        },
        type,
        delay);
  }

  void once(const std::function<void()>& callback, const pipeline type,
            const std::chrono::milliseconds delay)
  {
    schedule(
        [callback]()
        {
          callback();
          return cond_end;
        },
        type,
        delay);
  }

  class component final : public component_interface
  {
   public:
    void post_unpack() override
    {
      thread = utils::thread::create_named_thread(
          "Async Scheduler",
          []()
          {
            while (!kill)
            {
              execute(pipeline::async);
              std::this_thread::sleep_for(10ms);
            }
          });

      utils::hook::call(0x50CEDC, server_frame_stub);
    }

    void pre_destroy() override
    {
      kill = true;
      if (thread.joinable())
      {
        thread.join();
      }
    }
  };
} // namespace scheduler

REGISTER_COMPONENT(scheduler::component)
