#include <iostream>
#include <map>
#include <thread>
#include <string>
#include <mutex>
#include <list>
#include <condition_variable>
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>

#include "threadpool.h"

namespace async
{

class thread_pool_private
{
public:
    thread_pool_private (boost::asio::io_service& io,
                         bool take_io_controll,
                         unsigned count)
        : m_io (io)
        , m_stopFlag (false)
        , m_doneEmited (false)
    {
        if (take_io_controll)
            m_io_work.reset(new boost::asio::io_service::work (io));

        if (count == 0)
        { count = std::thread::hardware_concurrency(); }

        for (unsigned i = 0; i < count; ++i)
        {
            m_threads.emplace_back (std::thread ([&]()
            {
                while (1)
                {
                    if (m_stopFlag)
                    { return; }

                    std::unique_lock<std::mutex> lockInput (m_inputMutex);

                    if (m_inputTasks.empty())
                    { m_waitCond.wait (lockInput); }

                    if (m_stopFlag)
                    { return; }

                    if (!m_inputTasks.empty())
                    {
                        task_holder_ptr task = m_inputTasks.front();
                        m_inputTasks.pop_front();
                        lockInput.unlock();
                        task->run();
                        std::lock_guard<std::mutex> lockOutput (m_outputMutex);
                        m_doneTasks.emplace_back (std::move (task));

                        if (!m_doneEmited)
                        {
                            m_doneEmited = true;
                            m_io.post (std::bind (&async::thread_pool_private::checkDoneList, this));
                        }
                    }
                }
            }));
        }
    }

    void checkDoneList()
    {
        std::list<task_holder_ptr> doneTasks;
        {
            std::lock_guard<std::mutex> lockOutput (m_outputMutex);
            m_doneEmited = false;
            doneTasks.swap (m_doneTasks);
        }

        for (const std::shared_ptr<task_holder>& task: doneTasks)
        {
            task->done();
        }
    }

    task_holder_ptr run (const task& task)
    {
        auto task_pt = std::make_shared<task_holder> (task, m_io);

        {
            std::lock_guard<std::mutex> lock (m_inputMutex);
            m_inputTasks.push_back (task_pt);
        }
        m_waitCond.notify_one();
        return task_pt;
    }

    void stop()
    {
        m_stopFlag = true;
        m_waitCond.notify_all();
    }

    void waitAll()
    {
        for (std::thread & t : m_threads)
        {
            t.join();
        }
    }

    void stop_io()
    {
        if (m_io_work)
        {
            m_io.poll();
            m_io_work.reset();
        }
    }

private:
    std::mutex m_inputMutex;
    std::mutex m_outputMutex;
    std::list<task_holder_ptr> m_inputTasks;
    std::list<task_holder_ptr> m_doneTasks;
    std::condition_variable m_waitCond;
    std::list<std::thread> m_threads;
    boost::asio::io_service& m_io;
    bool m_doneEmited;
    bool m_stopFlag;
    std::unique_ptr<boost::asio::io_service::work> m_io_work;
};

thread_pool::thread_pool (boost::asio::io_service& io,
                          bool take_io_controll,
                          unsigned count)
    : d (new thread_pool_private (io, take_io_controll, count))
{
}

thread_pool::~thread_pool()
{
    d->stop();
    d->waitAll();
    d->stop_io();
}

task_holder_ptr thread_pool::run (const task& task)
{
    return d->run (task);
}

task_holder::task_holder (const task& task,
                          boost::asio::io_service& io)
    : m_done(false)
    , m_task (task)
    , m_io (io)
    , m_timer(0)
{
}

void task_holder::run()
{
    m_task();
}

void task_holder::wait(async_ctx ctx)
{
    if (m_done)
        return;

    boost::asio::steady_timer timer(m_io,  boost::asio::steady_timer::clock_type::duration::max());
    m_timer = &timer;
    boost::system::error_code ec;
    timer.async_wait (ctx[ec]);
}



void task_holder::done()
{
    std::cout << "Task holder done() thread id = " << std::this_thread::get_id() << std::endl;
    std::cout.flush();
    m_done = true;
    if (m_timer)
    {
        m_timer->cancel();
    }
}

void wait_all(const std::vector<task_holder_ptr>& tasks, async_ctx ctx)
{
    for (const task_holder_ptr & task: tasks)
    {
        task->wait(ctx);
    }
}


}
