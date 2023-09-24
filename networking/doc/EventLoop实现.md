在muduo中，`EventLoop`的实现在`muduo/net/EventLoop.{h/cpp}`文件中，其中`EventLoop.h`包含了`EventLoop`的数据成员定义和其接口的声明；`EventLoop.cpp`包含了`EventLoop`接口的具体实现。

# 关键数据成员

`EventLoop`中有几个重要的数据成员，它们的定义如下：
```cpp
// muduo/net/EventLoop.h
class Poller;
class TimerQueue;
class Channel;

class EventLoop {
  public:
  ... // 此处省略接口声明
  private:
  ... // 此处省略内部接口声明
  std::unique_ptr<Poller> poller_;
  std::unique_ptr<TimerQueue> timerQueue_;
  std::unique_ptr<Channel> wakeupChannel_;
  typedef std::vector<Channel*> ChannelList;
  ChannelList activeChannels_;
  Channel *currentActiveChannel;
};
```

从定义中我们可以发现，一个`EventLoop`由一个`Poller`、一个`TimerQueue`和多个`Channel`组成；`Poller` 可以被认为是一个负责轮询网络连接的数据结构，`TimerQueue` 可以被认为是一个处理`EventLoop`中定时事件的数据结构，`Channel` 可以被认为是一个封装好的数据结构，它主要被用来表示网络连接以及该连接上所发生的事件。

接下来我们详细分析`EventLoop`的关键数据成员，由于`Poller` 和`TimerQueue` 结构的定义中同样包含 `Channel` 类型的成员，因此为了降低理解的难度，我们首先分析`Channel`的实现。

## Channel

`Channel` 的实现在`muduo/net/Channel.{h/cpp}`文件中，其中`Channel.h` 包含了`Channel` 的数据成员定义以及接口的声明，`Channel.cpp` 包含了其接口具体的实现。

### Channel 关键数据成员

我们首先来看`Channel` 的关键数据成员：

```cpp
// muduo/net/Channel.h
class Channel {
  public:
  ... 
  private:
  ...
  const int fd_;
  int events_;
  int revents_;
  ...

  typedef std::function<void()> EventCallback;
  typedef std::function<void(Timestamp)> ReadEventCallback;
  
  ReadEventCallback readCallback_;
  EventCallback writeCallback_;
  EventCallback closeCallback_;
  EventCallback errorCallback_;
};
```

可以看到，`Channel` 本质上就是一个文件描述符(`const int fd_`) 和一系列事件的封装。其中`events_` 表示在`fd_`上我们所感兴趣的事件类型，而`revents_`则是实际在`fd_`上所发生的事件类型。

muduo 沿用了Unix系统下`poll/epoll`系统调用对事件类型的定义，并在此基础上做出了自己的组合：

```cpp
// muduo/net/Channel.cpp
...
const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;
...
```

在`poll/epoll`系统调用中，`POLLIN`标志位表示可以从文件描述符对应的文件中不阻塞地读取数据；`POLLPRI`标志位表示可以不阻塞地读取高优先级数据；`POLLOUT`表示可以向文件描述符对应的文件不阻塞地写普通数据。

因此，`kNoneEvent` 事件表示我们不能对文件描述符对应的文件无阻塞地做任何读写操作；`kReadEvent` 事件表示我们可以无阻塞地读取文件描述符对应的文件中的数据；`kWriteEvent` 事件表示我们可以无阻塞地向文件描述符对应的文件中写入数据。

`Channel` 的定义还包含了一系列回调函数成员：`{read/write/close/error}Callback_`；通过为`Channel` 对象设置回调函数成员的值，我们可以在事件发生时让程序执行自定义的处理逻辑。

### enable/disable 系列接口

`Channel` 向用户提供了一系列`enable/disable` 接口：

```cpp
// muduo/net/Channel.h

void enableReading() {  
  events_ |= kReadEvent;  
  update();  
}  
  
void disableReading() {  
  events_ &= ~kReadEvent;  
  update();  
}  
  
void enableWriting() {  
  events_ |= kWriteEvent;  
  update();  
}  
  
void disableWriting() {  
  events_ &= ~kWriteEvent;  
  update();  
}  
  
void disableAll() {  
  events_ = kNoneEvent;  
  update();  
}
```

其中`enable` 系列的接口将`events_` 成员与特定的事件类型做或运算，表示用户希望操作系统能够通知该文件上所发生的特定类型事件。例如，对`Channel { fd_ = 6, events_ = 0, revents_ = 0} `的对象调用`enableReading` 方法时，当操作系统polling发现`fd=6` 对应的文件中有数据可以被读取时，能够通知用户程序；对 `Channel { fd_ = 7, events_ = 0, revents_ = 0} `的对象调用`enableWriting` 方法时，当操作系统polling发现 `fd=7` 对应的文件可以被无阻塞的写入数据时，能够通知用户程序。

`disable` 系列的接口先将事件类型取反，再与`events_` 成员做与运算，表示用户希望操作系统不再通知该文件上所发生的特定类型事件。例如，对`Channel { fd_ = 6, events_ = kReadEvents, revents_ = 0} `的对象调用`disableReading` 方法时，当操作系统polling发现`fd=6` 对应的文件中有数据可以被读取时，不再通知用户程序；对 `Channel { fd_ = 7, events_ = kWriteEvents, revents_ = 0} `的对象调用`disableWriting` 方法时，当操作系统polling发现 `fd=7` 对应的文件可以被无阻塞的写入数据时，不再通知用户程序。

### 回调函数的设置

`Channel` 向用户提供了一系列用于设置回调函数的接口：

```cpp
// muduo/net/Channel.h

typedef std::function<void()> EventCallback;
typedef std::function<void(Timestamp)> ReadEventCallback;
  
void setReadCallback(ReadEventCallback cb) { readCallback_ = std::move(cb); }  
void setWriteCallback(EventCallback cb) { writeCallback_ = std::move(cb); }  
void setCloseCallback(EventCallback cb) { closeCallback_ = std::move(cb); }  
void setErrorCallback(EventCallback cb) { errorCallback_ = std::move(cb); }

```

`Channel` 支持的回调函数有两类：`EventCallback` 和 `ReadEventCallback`，它们之间唯一的区别在于用户程序调用`ReadEventCallback` 时需要传入可读事件发生时的时间戳。

### 事件的处理

当成员`fd_` 上有事件发生时，`Channel` 需要进行处理；处理事件的核心逻辑实现在`handleEventWithGuard` 函数中：

```cpp
// muduo/net/Channel.cpp
void Channel::handleEventWithGuard(Timestamp receiveTime) {
  eventHandling_ = true;
  LOG_TRACE << reventsToString();
  if ((revents_ & POLLHUP) && !(revents_ & POLLIN)) {
    if (logHup_) {
      LOG_WARN << "fd = " << fd_ << " Channel::handle_event() POLLHUP";
    }
    if (closeCallback_)
      closeCallback_();
  }

  if (revents_ & POLLNVAL) {
    LOG_WARN << "fd = " << fd_ << " Channel::handle_event() POLLNVAL";
  }

  if (revents_ & (POLLERR | POLLNVAL)) {
    if (errorCallback_)
      errorCallback_();
  }

  if (revents_ & (POLLIN | POLLPRI | POLLRDHUP)) {
    if (readCallback_)
      readCallback_(receiveTime);
  }

  if (revents_ & POLLOUT) {
    if (writeCallback_)
      writeCallback_();
  }

  eventHandling_ = false;
}
```

其中成员`revents_` 记录了由操作系统返回的，`fd_` 对应文件上所发生的事件。在与特定的标志位相与后，`Channel` 会调用由用户程序设置的回调函数，执行特定的事件处理逻辑。例如，`(revents_ & POLLHUP) && !(revents_ & POLLIN)` 为`true` 时表示该文件对应的连接已被挂断，并且没有数据需要读取；如果用户设置了`closeCallback`，则调用该函数从而执行用户自定义的文件关闭逻辑。

## Poller

`Poller` 是muduo 对`Poll/Epoll` 系统调用的一个通用封装，向外部提供一个轮询的接口，主要被`EventLoop` 用来监听连接上的I/O事件。

### Poller 关键数据成员

`Poller` 的定义比较简单，包含的成员有：

```cpp
// muduo/net/Poller.h
class Poller : noncopyable {
...
protected:
  typedef std::map<int, Channel*> ChannelMap;
  ChannelMap channels_;

private:
  EventLoop *ownerLoop_;
};
```

其中`channels_` 成员是一个`map<int, Channel*>`，记录了`int` 类型的`fd` 和其`Channel` 之间的映射关系；`ownerLoop_` 成员记录了该`Poller` 被哪个`EventLoop` 对象所使用。

### poll 接口

`poll` 接口是muduo对`Poll/Epoll` 系统调用的封装，它的函数原型是`Timestamp poll(int timeoutMs, vector<Channel*> *activeChannels)`，其中函数的返回值表示从系统调用返回时的时间戳，参数`timeoutMs` 表示`Poll/Epoll` 系统调用阻塞等待的毫秒数，参数`activeChannels` 是一个`Channel` 的动态数组，用于记录本次系统调用监听到的所发生的事件；

该接口在`Poller` 中是纯虚函数，具体的实现由`muduo/net/poller/PollPoller.cc` 和 `muduo/net/poller/EPollPoller.cc` 提供：

```cpp
// muduo/net/Poller.h
class Poller : noncopyable {
public:
  virtual Timestamp poll(int timeoutMs, vector<Channel*> *activeChannels);
};

// muduo/net/poller/PollPoller.cc
Timestamp PollPoller::poll(int timeoutMs, ChannelList *activeChannels) {
	int numEvents = ::poll(&*pollfds_.begin(), pollfds_.size(), timeoutMs);
	...
	Timestamp now = Timestamp::now();
	fillActiveChannels(numEvents, activeChannels);
	...
	return now;
}

// muduo/net/poller/EPollPoller.cc
Timestamp EpollPoller::poll(int timeoutMs, vector<Channel*> *activeChannels) {
	int numEvents = ::epoll_wait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()), timeoutMs);
	...
	Timestamp now = Timestamp::now();
	fillActiveChannels(numEvents, activeChannels);
	...
	return now;
}
```

考虑到`poll` 和 `epoll` 在性能和使用场景上的差异，接下来选择使用场景更广，性能更高的`EpollPoller` 的实现进行分析。

### EPollPoller 的关键数据成员

`EPollPoller` 继承自`Poller`，它的定义也非常简单：

```cpp
// muduo/net/poller/EpollPoller.cc
class EPollPoller : public Poller {
...
private:
	int epollfd_;
	std::vector<struct epoll_event> events_;
};
```

其中`epollfd_` 成员是指向`epoll instance`的文件描述符，`events_` 成员用于保存系统调用`epoll_wait` 监听到的所发生的事件，事件用`struct epoll_event` 表示。

通过`man epoll_wait` ，我们发现`struct epoll_event` 的定义如下：

```cpp
typedef union epoll_data {
  void *ptr;
  int fd;
  uint32_t u32;
  uint64_t u64;
} epoll_data_t;

struct epoll_event {
  uint32_t events;
  epoll_data_t data;
};
```

其中`events` 成员表示文件上所发生事件的具体类型，该成员的值由操作系统进行设置；`data` 成员的类型是联合类型，可以存储指针，文件的`fd` ，32位和64位无符号整数等类型的值；该成员的值不由操作系统设置，而是用户在调用`epoll_ctl` 时通过参数设置。

### update/remove 接口

通过前面的介绍，我们了解到，对于事件这个概念，`epoll` 系统调用操作的是`struct epoll_event` 类型的对象，而muduo中操作的是`Channel` 类型的对象，因此，`EpollPoller` 为用户提供了`updateChannel` 和 `removeChannel` 接口来完成两种类型的转换操作。

我们首先来看`updateChannel` 接口的实现：

```cpp
// muduo/net/poller/EpollPoller.cpp

namespace {
const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;
};

void EpollPoller::updateChannel(Channel *channel) {
	...
	const int index = channel->index();
	if (index == kNew || index == kDeleted) {
		if (index == kNew) { ... }
		else { ... }
		channel->set_index(kAdded);
		update(EPOLL_CTL_ADD, channel);
	} else {
		...
		if (channel->isNoneEvent()) {
			update(EPOLL_CTL_DEL, channel);
			channel->set_index(kDeleted);
		} else {
			update(EPOLL_CTL_MOD, channel);
		}
	}
}

void EpollPoller::update(int operation, Channel* channel) {
	struct epoll_event event;
	event.events = channel->events();
	event.data.ptr = channel;
	int fd = channel->fd();
	...
	if (::epoll_ctl(epollfd_, operation, fd, &event) < 0) { ... }
	...
}
```

可以看到，`updateChannel` 通过调用`update` 函数来实现`Channel` 到`struct epoll_event` 的转换。`update` 函数首先获取到`Channel` 的`events_` 成员，并将其设置为`struct epoll_event` 的 `events` 成员，设置用户感兴趣的文件上发生的事件类型；随后将`Channel` 的`fd_` 成员，用户选择的epoll 操作`operation` 和创建好的`struct epoll_event` 对象一起作为参数传入`epoll_ctl` 系统调用，通知操作系统执行用户最新的事件需求。

此外，`updateChannel` 中用到了之前未介绍的`Channel` 的`index` 成员。我们知道，`Channel` 是文件(`fd` )和其一系列事件参数的组合。因此，在`Channel` 的定义中，`index` 成员的含义是该`Channel` 之于该`EpollPoller poller instance` 的状态。状态的类型有：1. `kNew`，表示该文件上我们感兴趣的事件类型未被加入到`epoll instance` 中；2. `kAdded`，表示该文件上我们感兴趣的事件类型已经被加入到`epoll instance` 中；3. `kDeleted`，表示调用接口前该文件上我们感兴趣的事件类型已经被加入到`epoll instance` 中，但是现在我们对文件上发生的任何事件都不感兴趣了，所以它从`epoll instance` 中被移出。

介绍完`update` 函数和`index` 成员后，我们来看看`updateChannel` 接口所做的具体操作：
1. 当`Channel` 的状态是`kNew` 或者`kDeleted` 时，调用`update(EPOLL_CTL_ADD, channel)` 和 `set_index(kAdded)`：`kNew` 和`kDeleted` 都表示该文件上我们感兴趣的事件类型没有被加入到`epoll instance` 中，因此通过`EPOLL_CTL_ADD` 操作将其加入，并将`Channel` 的状态设置为`kAdded`；
2. 当`Channel` 的状态是`kAdded` 且 `events_` 成员的值不为`kNoneEvent` 时，调用`update(EPOLL_CTL_MOD, channel)` ：`kAdded` 表示该文件上我们感兴趣的事件类型已经被加入到`epoll instance` 中，`events_` 成员的值不为`kNoneEvent` ，表示我们感兴趣的事件类型可能有变化(比如从关注文件的可写事件变到关注文件的可读事件)，因此通过`EPOLL_CTL_MOD` 操作进行修改；
3. 当`Channel` 的状态是`kAdded` 且 `events_` 成员的值不为`kNoneEvent` 时，调用`update(EPOLL_CTL_MOD, channel)` 和 `set_index(kDeleted)` ：`kAdded` 表示该文件上我们感兴趣的事件类型已经被加入到`epoll instance` 中，`events_` 成员的值等于`kNoneEvent` ，表示我们对该文件上发生的任何事件都不感兴趣，因此通过`EPOLL_CTL_DEL` 操作将其从`epoll instance` 中删除，并将`Channel` 的状态设置为`kDeleted`。

我们接着来看看`removeChannel` 接口的实现：

```cpp
// muduo/net/poller/EpollPoller.cpp
void EPollPoller::removeChannel(Channel* channel) {
	...
	if (index == kAdded) { update(EPOLL_CTL_DEL, channel); }
	channel->set_index(kNew);
}
```

`removeChannel` 接口逻辑并不复杂，它所做的具体操作是：
1. 当`Channel` 的状态是`kAdded` 时，调用`update(EPOLL_CTL_DEL, channel)` ：`kAdded` 表示该文件上我们感兴趣的事件类型已经被加入到`epoll instance` 中，由于此时我们要从`Poller` 中移出`Channel` ，因此通过`EPOLL_CTL_DEL` 操作将该文件上我们感兴趣的事件类型从`epoll instance` 中删除；
2. 调用`set_index(kNew)`：表示该文件上我们感兴趣的事件类型未被加入到`epoll instance` 中。

我们发现，想要从`epoll instance` 中移除文件上我们感兴趣的事件类型有两种方式：
1. 将`Channel` 的`events` 类型改为`kNoneEvent` 并调用`updateChannel`，此时`Channel` 的状态会被设置为`kDeleted`；
2. 直接调用`removeChannel`，此时`Channel` 的状态会被设置为`kNew`。

这两种方式还有一个区别，那就是`removeChannel` 会将该`Channel` 从`Poller` 的`ChannelMap` 中移除，而第一种方式不会。

介绍完`Poller` 实现后，我们再来看看`TimerQueue` 的实现。

## TimerQueue 

顾名思义， `TimerQueue` 是muduo中的定时器队列，负责对定时器进行管理，我们接下来分析它的功能以及具体实现。

### TimerQueue 关键数据成员

`TimerQueue` 的定义较为复杂，包含的关键数据成员有：

```cpp
// muduo/net/TimerQueue.h
class Timer;
class TimerId;

class TimerQueue : noncopyable {
public:
  ...
private:
  ...
  EventLoop* loop_;
  const int timerfd_;
  Channel timerfdChannel_;
  std::set<std::pair<Timestamp, Timer*>> timers_;
  std::set<std::pair<Timer*, int64_t>> activeTimers_;
  std::set<std::pair<Timer*, int64_t>> cancelingTimers_;
};
```

其中`loop_` 成员表示该计时器队列所属的`EventLoop`，剩下的成员或多或少都与类型`Timer` 有关联，因此我们首先介绍muduo中`Timer` 的实现，随后回过头来介绍这些成员的含义。

## Timer

`Timer` 的中文翻译是定时器，在muduo中，它的功能是：当到达了它的过期时间时，用户程序会收到通知，并执行相关操作。

为了完成这样的功能，我们来看看`Timer` 的具体实现。

### Timer 关键数据成员

`Timer` 包含的关键数据成员有：

```cpp
// muduo/net/Timer.h
typedef std::function<void()> TimerCallback;
class Timer : noncopyable {
public:
  ...
private:
  ...
  const TimerCallback callback_;
  Timestamp expiration_;
  const double interval_;
  const bool repeat_;
  ...
};
```

其中，`expiration_` 成员表示`Timer` 的到期时间，`callback_` 成员表示定时器到期时，用户程序所执行的回调函数。
muduo除了为`Timer` 赋予单次计时的功能以外，还提供了周期计时的功能，`repeat_` 成员表示该`Timer` 是否启用周期计时功能，`interval_` 成员表示计时周期的具体时长，单位为毫秒。

### Timer 的创建与周期计时功能

```cpp
// muduo/net/Timer.h
class Timer : noncopyable {
public:
  Timer(TimerCallback cb, Timestamp when, double interval) :
  callback_(cb), expiration_(when), interval_(interval), repeat_(interval > 0.0) ... { ... }
  ...
};

// muduo/net/Timer.cpp
void Timer::restart(Timestamp now) {
	if (repeat_) {
		expiration_ = addTime(now, interval_);
	} else {
		expiration_ = Timestamp::invalid();
	}
}
```

观察`Timer` 的构造函数，我们发现，`Timer` 是单次计时还是周期计时由传入的参数`interval` 决定，当 `interval > 0` 时，`repeat_` 成员被设置为`true`  ，`Timer` 自动启用周期计时功能。

周期计时的功能主要由`restart` 接口实现，首先它会判断`Timer` 是否启用周期计时功能，如果启用，则将`Timer` 的超时时间设置为`now + interval` ，表示在当前时刻过`interval` 毫秒后，`Timer` 将再次超时；如果未启用，则将`Timer` 的超时时间设置为 `invalid`，表示该`Timer` 未来将不会再超时。

### Timer 的 id

为了区分每个`Timer` 对象，muduo为其维护了一个`id`，可以通过`Timer` 的构造函数查看其具体实现：

```cpp
// muduo/net/Timer.h
class Timer : noncopyable {
public:
	Timer(TimerCallback cb, Timestamp when, double interval)
	 : callback_(cb), expiration_(when), interval_(interval), repeat_(interval > 0.0) sequence_(s_numCreated_.getAndIncrement()) { ... }
private:
	...
	const int64_t sequence_;
	static AtomicInt64 s_numCreated_;
};
```

`Timer` 对象的`id` 由`sequence_` 成员表示，类型为`int64_t`，值通过访问静态原子成员`s_numCreated_` 而得到。

### 再看 TimerQueue 关键数据成员

在了解完`Timer` 的实现后，我们知道，一个`Timer` 类型的对象对应着一个超时时间点以及超时后的回调函数，此外，如果开启周期计时功能，则该对象还记录了它超时周期的时长。

```cpp
// muduo/net/TimerQueue.h
class Timer;
class TimerId;

class TimerQueue : noncopyable {
public:
  ...
private:
  ...
  EventLoop* loop_;
  const int timerfd_;
  Channel timerfdChannel_;
  std::set<std::pair<Timestamp, Timer*>> timers_;
  std::set<std::pair<Timer*, int64_t>> activeTimers_;
  std::set<std::pair<Timer*, int64_t>> cancelingTimers_;
};
```

此时我们已经可以理解`TimerQueue`中剩余成员的含义了，其中`timers_` 表示`TimerQueue` 中所有的`Timer` 以及它们对应的超时时间，`activeTimer_` 表示`TimerQueue` 中所有未过期的`Timer`以及它们对应的`id` , `cancelingTimers_` 表示用户请求取消的`Timer` 以及它们对应的`id`。

我们需要思考的最后一个问题是，`timerfd_` 成员用于指向什么文件，为什么要关注这个文件上所发生的事件？

### Linux 定时器相关的系统调用

通过观察`TimerQueue` 的构造函数，我们发现，`timerfd_` 成员的值是系统调用`timerfd_create` 的返回值：

```cpp
// muduo/net/Timer.cpp

int createTimerfd()
{
	int timerfd = ::timerfd_create(CLOCK_MONOTONIC,TFD_NONBLOCK | TFD_CLOEXEC);
	...
	return timerfd;
}

TimerQueue::TimerQueue(EventLoop* loop)
: loop_(loop),
timerfd_(createTimerfd()),
timerfdChannel_(loop, timerfd_),
timers_(),
callingExpiredTimers_(false)
{
	...
}
```

通过查看手册`man timerfd_create` ，我们发现，该系统调用主要用于创建一个内核`Timer` 对象，并且它返回一个指向该对象的文件描述符。我们向该系统调用传递了两个参数：`CLOCK_MONOTONIC` 和 `TFD_NONBLOCK | TFD_CLOSEXEC`，其中`CLOCK_MONOTONIC` 表示系统选择的时钟从系统启动后的某个时间点开始，一直是单调递增的，`TFD_NONBLOCK` 表示对该文件描述符的I/O 操作是非阻塞的，`TFD_CLOSEXEC` 表示指向该对象的文件描述符会在程序执行`exec` 系统调用的时候被关闭。

系统调用`timerfd_settime` 负责设置内核`Timer` 对象的超时时间，它的原型是`int timerfd_settime(int fd, int flags, const struct itimerspec *new_value, struct itimerspec *old_value)`，`struct itimerspec` 有两个关键成员：`it_value` 和 `it_interval`，`it_value` 表示内核`Timer` 超时的时间点，`it_interval` 表示内核`Timer` 超时的周期，它们的类型都是`struct timespec`，其中`tv_sec` 表示某时间点的秒数，`tv_nsec` 表示某时间点的纳秒数。

由于系统调用仅仅返回了一个文件描述符，因此内核`Timer` 对象的超时通知也得由用户程序通过该文件描述符获取。手册详细描述了通知的获取流程：
1. 如果内核`Timer` 已经超时，那么调用`int read(int fd, void* buf, size_t count)` 时，`buf` 中会存储八个字节，解释成`uint64_t`时即代表内核`Timer` 超时的次数；
2. 如果内核`Timer` 未超时，那么调用`int read(int fd, void* buf, size_t count)` 时，如果文件描述符不是`NONBLOCKING`的，则该调用阻塞到内核`Timer` 下次超时为止；如果文件描述符是`NONBLOCKING`的，则该调用立即返回，且`errno` 被设置为 `EAGAIN`。

此外，`timerfd_create` 返回文件描述符的另一个重要原因是，该文件描述符可以被`EPollPoller` 所监听；当内核`Timer` 超时时，`EPollPoller` 可以监听到文件描述符上的`kReadEvent` 事件，用户程序通过处理事件执行自定义的处理逻辑。

在了解完`timerfd` 相关的系统调用之后，我们可以回答上一节留下的问题了：`timerfd_` 指向的是内核`Timer` 对象，`timerfdChannel_` 是`timerfd_` 和一系列事件参数的封装；我们主要关注的是`timerfd_` 上的`kReadEvent` 事件，该事件意味着内核`Timer` 已经超时，用户程序可以通过处理事件执行后续的逻辑。

### addTimer 与 cancel

`TimerQueue` 向用户提供了`addTimer` 和 `cancel` 两个接口，用于向其中添加和取消`Timer` 对象，我们首先来看`addTimer` 的实现：

```cpp
// muduo/net/TimerQueue.cpp

TimerId TimerQueue::addTimer(TimerCallback cb, Timestamp when, double interval) {
	Timer* timer = new Timer(std::move(cb), when, interval);
	loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timer));
	return TimerId(timer, timer->sequence());
}

void TimerQueue::addTimerInLoop(Timer* timer) {
	...
	bool earliestChanged = insert(timer);
	if (earliestChanged) {
		resetTimerfd(timerfd_, timer->expiration());
	}
}
```

`addTimer` 是面向用户的接口，主要的逻辑实现在`addTimerInLoop` 中：它首先调用`insert` 函数将`Timer` 添加到`timers_` 和 `activeTimers_` 集合中，并判断`TimerQueue` 的最早超时时间是否发生变化。由于`timers_` 和 `activeTimers_` 皆为以`Timer` 超时时间排序的有序集合，因此在插入时只需要比较待插入`Timer` 和有序集合中第一个`Timer` 的超时时间即可判断最早超时时间是否发生变化。

当最早超时时间发生变化时，`addTimerInLoop` 调用`resetTimerfd` 函数来重新设置内核`Timer` 的超时时间，该函数调用了之前提到的`timerfd_settime` 系统调用。

我们接着来看`cancel` 的实现：

```cpp
// muduo/net/TimerQueue.cpp
void TimerQueue::cancel(TimerId timerId)
{
	loop_->runInLoop(std::bind(&TimerQueue::cancelInLoop, this, timerId));
}

void TimerQueue::cancelInLoop(TimerId timerId) {
	...
	std::pair<Timer*, int64_t> timer(timerId.timer_, timerId.sequence_);
	auto it = activeTimers_.find(timer);
	if (it != activeTimers_.end()) {
		timers_.erase(Entry(it->first->expiration(), it->first));
		activeTimers_.erase(it);
		...
	}
}
```

当用户程序想要取消一个`Timer` 对象时，它需要向`cancel` 接口传递一个`TimerId` ，其中包括`Timer` 对象的指针以及`Timer` 对象的`id`。`cancelInLoop` 首先判断该`Timer` 是否位于`activeTimers_` 集合中， 确保只有没有超时的`Timer` 对象才会被取消。当待取消的`Timer` 对象没有超时时，它将被从`activeTimers_` 和`timers_` 集合中删除。

### 超时处理

除了`addTimer` 和 `cancel` 接口以外，`TimerQueue` 还有一个很重要的功能便是处理`Timer` 的超时事件，接下来我们详细追踪一下当`Timer` 超时时，它的超时事件是如何被处理的。

在向`TimerQueue` 插入`Timer` 时，它会去检查并设置内核`Timer` 的超时时间；因此，当`Timer` 对象超时时，内核`Timer` 对应的`timerfd_` 一定是可读的，`Timer` 的超时事件处理流程从读取`timerfd_` 开始，具体的逻辑实现在`handleRead` 函数中：

```cpp
// muduo/net/TimerQueue.cpp
void TimerQueue::handleRead()
{
	...
	readTimerfd(timerfd_, now);
	std::vector<std::pair<Timestamp, Timer*>> expired = getExpired(now);
	// safe to callback outside critical section
	for (const Entry& it : expired)
	{
		it.second->run();
	}
	reset(expired, now);
}
```

`handleRead` 首先调用`readTimerfd` 函数，获取内核`Timer` 超时的次数，随后调用`getExpired` 函数获取当前时刻`timers_` 集合中所有超时的`Timer` 对象，通过`run` 方法调用其绑定的超时回调方法，最后调用`reset` 函数重新设置`Timer` 和内核`Timer` 下一次的超时时间。

`getExpired` 函数实现如下：

```cpp
// muduo/net/TimerQueue.cpp
std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now) {
	std::vector<std::pair<Timestamp, Timer*>> expired;
	std::pair<Timestamp, Timer*> sentry(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
	auto end = timers_.lower_bound(sentry);
	...
	std::copy(timers_.begin(), end, back_inserter(expired));
	timers_.erase(timers_.begin(), end);


	for (const Entry& it : expired)
	{
		std::pair<Timer*, int64_t> timer(it.second, it.second->sequence());
		size_t n = activeTimers_.erase(timer);
		...
	}
	...
	return expired;
}
```

它首先通过`lower_bound(Timestamp::now())` 函数找到`timers_` 集合中所有过期时间小于当前时刻，即已经超时的`Timer` 对象，并放到`expired` 数组中保存，随后将这些超时的`Timer` 对象从`activeTimers_` 集合中删除，最后将`expired` 数组作为返回值返回。

`reset` 函数实现如下：

```cpp
void TimerQueue::reset(const std::vector<Entry>& expired, Timestamp now) {
	Timestamp nextExpire;
	for (const Entry& it : expired) {
		std::pair<Timer*, int64_t> timer(it.second, it.second->sequence());
		if (it.second->repeat()) {
			it.second->restart(now);
			insert(it.second);
		}
		else {
			delete it.second;
		}
	}
	if (!timers_.empty()) {
		nextExpire = timers_.begin()->second->expiration();
	}
	
	if (nextExpire.valid() {
		resetTimerfd(timerfd_, nextExpire);
	}
}
```

`reset` 函数首先处理的是超时`Timer` 数组`expired`，对于那些开启了周期计时的`Timer`，`reset` 函数设置它们下次超时的时间，并重新插入到`timers_` 和 `activeTimers_` 集合中；对于那些单次计时的`Timer`，`reset` 函数直接释放它们的资源。随后，`reset` 函数将`timers_` 有序集合中第一个`Timer` 的超时时间作为下一次超时时间设置给内核`Timer`。

至此，我们介绍完了`EventLoop` 的关键成员`Channel`，`Poller`和`TimerQueue` 实现细节，接下来我们可以将目光放回到`EventLoop` 的实现上了。

### 事件通知与处理

muduo 默认使用`epoll`系列系统调用来请求操作系统监测文件上所发生的事件，具体使用的系统调用在`muduo/net/poller/DefaultPoller.cpp` 文件中确定：

```cpp
Poller *Poller::newDefaultPoller(EventLoop *loop) {  
  if (::getenv("MUDUO_USE_POLL")) {  
    return new PollPoller(loop);  
  } else {  
    return new EPollPoller(loop);  
  }  
}
```

当 `epoll_wait` 系统调用返回时，操作系统将用户关注的文件上所发生的事件返回给用户程序：

```cpp
// muduo/net/poller/EPollPoller.cc

Timestamp EPollPoller::poll(...) {
	...
	int numEvents = ::epoll_wait(epollfd_, &*events_.begin(), events_.size(), timeoutMs);
	
}
```

其中`numEvents` 表示事件发生的个数，`events` 链表记录了所有发生事件的详细信息。