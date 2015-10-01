#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <memory>
#include <functional>
#include <boost/asio/spawn.hpp>
#include <boost/asio/steady_timer.hpp>

namespace boost
{
namespace asio
{
class io_service;
}
}

namespace async
{

typedef boost::asio::yield_context async_ctx;

typedef std::function<void() > task;

class thread_pool_private;

class task_holder
    : boost::noncopyable
{
public:
    task_holder (const task& task,
                 boost::asio::io_service& io);

    void run();

    void set_async_timer (boost::asio::steady_timer& timer);

private:
    friend class thread_pool_private;

    void done();

    task m_task;
    boost::asio::io_service& m_io;
    boost::asio::steady_timer* m_timer;
};
typedef std::shared_ptr<task_holder> task_holder_ptr;

class thread_pool
    : boost::noncopyable
{
public:
    thread_pool (boost::asio::io_service& io,
                 bool take_io_controll = false,
                 unsigned count = 0);

    void run_and_wait (const task& task, async_ctx ctx);

    void run (const task& task);

    ~thread_pool();

private:
    std::unique_ptr<thread_pool_private> d;

};

}


#endif // THREADPOOL_H
