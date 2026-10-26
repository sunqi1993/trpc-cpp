# tRPC-Cpp Fiber 体系架构

## 一、全景架构：Fiber 在 tRPC-Cpp 中的位置

```mermaid
graph TB
    subgraph Application ["应用层 Application"]
        App1["用户业务代码"]
        App2["gRPC/HTTP 服务"]
        App3["Redis/MySQL 客户端"]
    end

    subgraph RPC ["RPC 层"]
        RPC1["Server 端"]
        RPC2["Client 端"]
        RPC3["Streaming RPC"]
        RPC4["Filter Chain"]
    end

    subgraph Runtime ["运行时层 Runtime"]
        subgraph ThreadModel ["线程模型"]
            TM1["Separate 独立模型"]
            TM2["Merge 合并模型"]
            TM3["Fiber M:N 协程 ★"]
        end

        subgraph IOModel ["IO 模型"]
            IO1["Reactor 网络 IO"]
            IO2["AsyncIO io_uring"]
        end

        subgraph Scheduling ["调度系统"]
            SG["SchedulingGroup"]
            FW["FiberWorker OS Thread"]
            TW["TimerWorker"]
            Sch["Scheduling V1/V2"]
        end
    end

    subgraph Transport ["传输层 Transport"]
        T1["TCP/UDP/Unix Socket"]
        T2["SSL/TLS"]
        T3["Codec 协议编解码"]
    end

    subgraph Plugins ["插件层 Plugins"]
        P1["Naming 服务发现"]
        P2["Config 配置管理"]
        P3["Logging 日志"]
        P4["Metrics 监控"]
        P5["Tracing 链路追踪"]
    end

    subgraph OS ["操作系统层"]
        OS1["pthreads"]
        OS2["epoll/io_uring"]
        OS3["mmap"]
        OS4["dlsym"]
    end

    App1 --> RPC1
    App2 --> RPC1
    App3 --> RPC2

    RPC1 --> Transport
    RPC2 --> Transport
    RPC3 --> Transport
    RPC4 --> Runtime

    Runtime --> OS
    Transport --> OS
    Plugins --> Runtime

    TM3 --> SG
    SG --> FW
    SG --> TW
    SG --> Sch

    classDef app fill:#e3f2fd,stroke:#1565c0,stroke-width:3px
    classDef rpc fill:#f3e5f5,stroke:#7b1fa2,stroke-width:2px
    classDef runtime fill:#fff3e0,stroke:#e65100,stroke-width:3px
    classDef transport fill:#e8f5e8,stroke:#2e7d32,stroke-width:2px
    classDef plugin fill:#fce4ec,stroke:#c62828,stroke-width:2px
    classDef os fill:#eceff1,stroke:#546e7a,stroke-width:2px

    class App1,App2,App3 app
    class RPC1,RPC2,RPC3,RPC4 rpc
    class TM1,TM2,TM3,IO1,IO2,SG,FW,TW,Sch runtime
    class T1,T2,T3 transport
    class P1,P2,P3,P4,P5 plugin
    class OS1,OS2,OS3,OS4 os
```

## 二、层次架构：从用户到硬件

```mermaid
graph LR
    subgraph L1 ["Level 1: 用户接口"]
        direction TB
        L1A["Fiber() / StartFiberDetached()"]
        L1B["FiberLatch / FiberConditionVariable"]
        L1C["FiberMutex / FiberEvent"]
        L1D["FiberLocal fiber-local storage"]
    end

    subgraph L2 ["Level 2: 调度抽象"]
        direction TB
        L2A["SchedulingGroup 调度组"]
        L2B["Scheduling 调度器接口"]
        L2C["FiberWorker OS 线程"]
        L2D["TimerWorker 定时器"]
    end

    subgraph L3 ["Level 3: 执行引擎"]
        direction TB
        L3A["FiberEntity fiber 控制块"]
        L3B["FiberDesc fiber 描述符"]
        L3C["StackAllocator 栈分配"]
        L3D["fcontext 汇编上下文切换"]
    end

    subgraph L4 ["Level 4: 内存管理"]
        direction TB
        L4A["MemoryPool 内存池"]
        L4B["mmap 系统调用"]
        L4C["Guard Page 保护页"]
        L4D["ObjectPool 对象池"]
    end

    subgraph L5 ["Level 5: 硬件"]
        direction TB
        L5A["CPU Registers"]
        L5B["Cache Line"]
        L5C["Virtual Memory"]
        L5D["TLB"]
    end

    L1 --> L2
    L2 --> L3
    L3 --> L4
    L4 --> L5

    classDef l1 fill:#e3f2fd,stroke:#1565c0,stroke-width:3px
    classDef l2 fill:#f3e5f5,stroke:#7b1fa2,stroke-width:3px
    classDef l3 fill:#fff3e0,stroke:#e65100,stroke-width:3px
    classDef l4 fill:#e8f5e8,stroke:#2e7d32,stroke-width:3px
    classDef l5 fill:#eceff1,stroke:#546e7a,stroke-width:3px

    class L1A,L1B,L1C,L1D l1
    class L2A,L2B,L2C,L2D l2
    class L3A,L3B,L3C,L3D l3
    class L4A,L4B,L4C,L4D l4
    class L5A,L5B,L5C,L5D l5
```

## 三、核心组件关系图

```mermaid
graph TB
    subgraph Core ["核心组件"]
        SG["SchedulingGroup"]
        FW["FiberWorker"]
        TW["TimerWorker"]
        Sch["Scheduling"]
        FE["FiberEntity"]
        FD["FiberDesc"]
        SA["StackAllocator"]
        JC["jump_context"]
    end

    subgraph Queue ["队列系统"]
        GQ["Global RunQueue"]
        LQ["Local RunQueue V2"]
        TQ["Timer Queue"]
        WQ["Wait Queue"]
    end

    subgraph Synchron ["同步原语"]
        FL["FiberLatch"]
        FCV["FiberConditionVariable"]
        FE2["FiberEvent"]
        FM["FiberMutex"]
        SP["Spinlock"]
        WL["Waitable"]
    end

    SG -->|"包含"| FW
    SG -->|"包含"| TW
    SG -->|"委托"| Sch
    SG -->|"管理"| FE

    FW -->|"调度循环"| SG
    TW -->|"定时器循环"| SG
    Sch -->|"V1: 全局队列"| GQ
    Sch -->|"V2: 本地队列"| LQ
    TW -->|"管理"| TQ

    FE -->|"存储在栈顶"| SA
    FE -->|"上下文切换"| JC
    FE -->|"包含"| FD
    FE -->|"等待链"| WL

    FL -->|"计数器同步"| WQ
    FCV -->|"条件等待"| WQ
    FE2 -->|"事件通知"| WQ
    FM -->|"互斥锁"| SP
    FM -->|"等待"| WQ

    classDef core fill:#fff3e0,stroke:#e65100,stroke-width:3px
    classDef queue fill:#e8eaf6,stroke:#283593,stroke-width:2px
    classDef sync fill:#e8f5e8,stroke:#2e7d32,stroke-width:2px

    class SG,FW,TW,Sch,FE,FD,SA,JC core
    class GQ,LQ,TQ,WQ queue
    class FL,FCV,FE2,FM,SP,WL sync
```

## 四、Fiber 调度数据流

```mermaid
flowchart TB
    Start(["用户调用 StartFiberDetached()"])
    CreateFiber["创建 FiberDesc"]
    PushQueue["推入 Global RunQueue"]
    Schedule["SchedulingGroup.Schedule()"]
    Acquire["FiberWorker.AcquireFiber()"]
    CheckLocal{"V2: 检查 LocalQueue"}
    CheckGlobal["检查 Global RunQueue"]
    Steal["从其他 Worker 偷取"]
    Restore["jump_context 恢复上下文"]
    Execute["执行 Fiber 代码"]
    Yield{"FiberYield()?"}
    Sleep{"等待事件?"}
    Wake["被唤醒 Resume"]
    Dead["Fiber 退出"]
    Free["释放 FiberEntity"]

    Start --> CreateFiber
    CreateFiber --> PushQueue
    PushQueue --> Schedule
    Schedule --> Acquire
    Acquire --> CheckLocal
    CheckLocal -->|"有任务"| Restore
    CheckLocal -->|"无任务"| CheckGlobal
    CheckGlobal -->|"有任务"| Restore
    CheckGlobal -->|"无任务"| Steal
    Steal -->|"成功"| Restore
    Steal -->|"失败"| Acquire
    Restore --> Execute
    Execute --> Yield
    Yield -->|"是"| PushQueue
    Yield -->|"否"| Sleep
    Sleep -->|"是"| Wake
    Sleep -->|"否"| Execute
    Wake --> Execute
    Execute --> Dead
    Dead --> Free

    classDef start fill:#c8e6c9,stroke:#2e7d32,stroke-width:3px
    classDef process fill:#e3f2fd,stroke:#1565c0,stroke-width:2px
    classDef decision fill:#fff3e0,stroke:#e65100,stroke-width:2px
    classDef end fill:#ffcdd2,stroke:#c62828,stroke-width:3px

    class Start start
    class CreateFiber,PushQueue,Schedule,Acquire,Restore,Execute,Wake process
    class CheckLocal,Sleep,Yield decision
    class Dead,Free end
```

## 五、Fiber 状态机与上下文切换

```mermaid
stateDiagram-v2
    [*] --> Created: StartFiberDetached()
    Created --> Ready: Push to RunQueue

    state Ready {
        [*] --> GlobalQueue
        GlobalQueue --> LocalQueue: V2 work stealing
    }

    state Running {
        [*] --> UserCode
        UserCode --> jump_context: 调用 jump_context
        jump_context --> SaveRegisters: 保存 callee-saved 寄存器
        SaveRegisters --> SwitchStack: 切换栈指针
    }

    state Yield {
        [*] --> Requeue
        Requeue --> PushGlobal: 推入 Global RunQueue
    }

    state Waiting {
        [*] --> WaitChain
        WaitChain --> SchedulerLock: 持有 scheduler_lock
        SchedulerLock --> StateTransition: 状态转换
    }

    Ready --> Running: Worker 调度
    Running --> Yield: FiberYield()
    Running --> Waiting: 等待事件
    Running --> Dead: 协程退出
    Yield --> Ready: 重新入队
    Waiting --> Ready: 被唤醒 Resume
    Dead --> [*]: 释放资源

    note right of Ready
        V1: 全局队列
        V2: 本地队列 + work stealing
    end note

    note right of Running
        jump_context 保存/恢复
        AArch64: x19-x30, d8-d15
        x86_64: rbx, rbp, r12-r15
    end note

    note right of Waiting
        scheduler_lock 保护
        防止并发唤醒竞争
    end note
```

## 六、跨组件交互时序

```mermaid
sequenceDiagram
    participant User as 用户代码
    participant Fiber as Fiber API
    participant SG as SchedulingGroup
    participant Sch as Scheduling
    participant FW as FiberWorker
    participant FE as FiberEntity
    participant JC as jump_context
    participant OS as OS pthreads

    User->>Fiber: StartFiberDetached(proc)
    Fiber->>SG: StartFiber(desc)
    SG->>Sch: StartFiber(desc)
    Sch->>Sch: Push to RunQueue

    Note over FW: Worker 循环
    loop 调度循环
        FW->>SG: Schedule()
        SG->>Sch: AcquireFiber()
        Sch->>FE: 获取 FiberEntity
        FE->>FE: 设置 state = Running
        FW->>FE: Resume()
        FE->>JC: jump_context()
        JC->>OS: 保存寄存器, 切换栈
        Note over FE: 执行 Fiber 代码
    end

    alt FiberYield()
        FE->>SG: Yield(self)
        SG->>Sch: Push to RunQueue
        FE->>JC: jump_context()
        JC->>OS: 恢复调用者上下文
    else 等待事件
        FE->>FE: state = Waiting
        FE->>SG: Suspend(self)
        Note over FE: 加入 WaitChain
        FE->>JC: jump_context()
        JC->>OS: 恢复调用者上下文
    end

    Note over FW: 其他 Fiber 可能被调度

    alt 被唤醒
        Note over SG: 事件触发
        SG->>FE: Resume(fiber)
        FE->>FE: state = Ready
        FE->>Sch: Push to RunQueue
    end
```

## 七、内存布局与栈管理

```mermaid
graph TB
    subgraph VirtualMemory ["虚拟内存布局"]
        direction TB
        High["高地址 栈顶"]
        
        subgraph Stack ["Fiber 栈 可增长"]
            Magic["Magic: TRPC_FIBER_ENTITY 64B"]
            Entity["FiberEntity cache-line 对齐"]
            StackTop["state_save_area 栈顶"]
            UserStack["用户栈空间"]
        end
        
        Guard1["Guard Page 1 不可读写"]
        Bottom["栈底"]
        Guard2["Guard Page 2 不可读写"]
        
        Low["低地址 栈底"]
    end

    subgraph Components ["组件说明"]
        C1["Magic: 识别 fiber 栈的魔数"]
        C2["FiberEntity: 控制结构 256B"]
        C3["state_save_area: 上下文切换时保存寄存器"]
        C4["用户栈空间: 协程执行代码"]
        C5["Guard Page: 检测栈溢出"]
    end

    High --> Stack
    Stack --> Guard1
    Guard1 --> Bottom
    Bottom --> Guard2
    Guard2 --> Low

    C1 -.-> Magic
    C2 -.-> Entity
    C3 -.-> StackTop
    C4 -.-> UserStack
    C5 -.-> Guard1

    classDef memory fill:#e8f5e8,stroke:#2e7d32,stroke-width:2px
    classDef guard fill:#ffcdd2,stroke:#c62828,stroke-width:2px
    classDef component fill:#e3f2fd,stroke:#1565c0,stroke-width:2px

    class Magic,Entity,StackTop,UserStack memory
    class Guard1,Guard2 guard
    class C1,C2,C3,C4,C5 component
```

## 八、V1 vs V2 调度器对比

```mermaid
graph TB
    subgraph V1 ["V1: 全局 RunQueue"]
        direction TB
        V1G["Global RunQueue 全局队列"]
        V1W1["Worker 1"]
        V1W2["Worker 2"]
        V1WN["Worker N"]

        V1W1 -->|"push/pop"| V1G
        V1W2 -->|"push/pop"| V1G
        V1WN -->|"push/pop"| V1G

        V1Desc["特点: 实现简单 单点竞争 适合小规模"]
    end

    subgraph V2 ["V2: 全局 + 本地 LocalQueue"]
        direction TB
        V2G["Global RunQueue 全局队列"]
        V2L1["Local Queue 1 本地队列"]
        V2L2["Local Queue 2 本地队列"]
        V2LN["Local Queue N 本地队列"]
        V2W1["Worker 1"]
        V2W2["Worker 2"]
        V2WN["Worker N"]

        V2W1 -->|"优先 push/pop"| V2L1
        V2W2 -->|"优先 push/pop"| V2L2
        V2WN -->|"优先 push/pop"| V2LN

        V2L1 -.->|"work stealing"| V2G
        V2L2 -.->|"work stealing"| V2G
        V2LN -.->|"work stealing"| V2G

        V2G -.->|"远程 steal"| V2L1
        V2G -.->|"远程 steal"| V2L2
        V2G -.->|"远程 steal"| V2LN

        V2Desc["特点: 减少竞争 支持 work stealing 适合大规模"]
    end

    classDef v1 fill:#fff3e0,stroke:#e65100,stroke-width:2px
    classDef v2 fill:#e8eaf6,stroke:#283593,stroke-width:2px

    class V1G,V1W1,V1W2,V1WN,V1Desc v1
    class V2G,V2L1,V2L2,V2LN,V2W1,V2W2,V2WN,V2Desc v2
```

## 九、Fiber 与线程模型对比

```mermaid
graph LR
    subgraph ThreadModel ["传统线程模型"]
        direction TB
        T1["OS Thread 1"]
        T2["OS Thread 2"]
        T3["OS Thread N"]

        T1 -->|"1:1"| Task1["任务 1"]
        T2 -->|"1:1"| Task2["任务 2"]
        T3 -->|"1:1"| TaskN["任务 N"]
    end

    subgraph FiberModel ["Fiber M:N 模型"]
        direction TB
        FW1["FiberWorker 1"]
        FW2["FiberWorker 2"]
        FW3["FiberWorker N"]

        F1["Fiber 1"]
        F2["Fiber 2"]
        F3["Fiber 3"]
        F4["Fiber 4"]
        F5["Fiber 5"]
        FN["Fiber M"]

        F1 --> FW1
        F2 --> FW1
        F3 --> FW2
        F4 --> FW2
        F5 --> FW3
        FN --> FW3
    end

    subgraph Comparison ["对比"]
        C1["线程: 1:1 映射 上下文切换开销大 内存占用高"]
        C2["Fiber: M:N 映射 上下文切换开销小 内存占用低 支持 work stealing"]
    end

    ThreadModel ~~~ Comparison
    FiberModel ~~~ Comparison

    classDef thread fill:#ffcdd2,stroke:#c62828,stroke-width:2px
    classDef fiber fill:#c8e6c9,stroke:#2e7d32,stroke-width:2px
    classDef compare fill:#e3f2fd,stroke:#1565c0,stroke-width:2px

    class T1,T2,T3,Task1,Task2,TaskN thread
    class FW1,FW2,FW3,F1,F2,F3,F4,F5,FN fiber
    class C1,C2 compare
```

## 十、关键数据结构关系

```mermaid
classDiagram
    class SchedulingGroup {
        +FiberWorker[] workers
        +TimerWorker timer_worker
        +Scheduling scheduling
        +Schedule()
        +StartFiber()
        +Yield()
        +Resume()
    }

    class FiberWorker {
        +SchedulingGroup sg
        +size_t worker_index
        +thread worker
        +WorkerProc()
        +AddForeignSchedulingGroup()
    }

    class TimerWorker {
        +SchedulingGroup sg
        +priority_queue timers
        +thread worker
        +CreateTimer()
        +EnableTimer()
    }

    class Scheduling {
        <<abstract>>
        +Init()
        +Schedule()
        +AcquireFiber()
        +StartFiber()
        +Suspend()
        +Resume()
    }

    class FiberEntity {
        +FiberState state
        +Spinlock scheduler_lock
        +SchedulingGroup* scheduling_group
        +void* state_save_area
        +size_t stack_size
        +Resume()
        +ResumeOn()
    }

    class FiberDesc {
        +size_t stack_size
        +Function start_proc
        +bool scheduling_group_local
    }

    class StackAllocator {
        +Allocate()
        +Deallocate()
    }

    class Waitable {
        +DoublyLinkedList waiters
        +AddWaiter()
        +RemoveWaiter()
    }

    SchedulingGroup *-- FiberWorker : contains
    SchedulingGroup *-- TimerWorker : contains
    SchedulingGroup *-- Scheduling : delegates
    SchedulingGroup *-- FiberEntity : manages
    FiberEntity *-- FiberDesc : contains
    FiberEntity --> StackAllocator : allocates
    FiberEntity --> Waitable : waits
    Scheduling --> FiberEntity : schedules
```

## 十一、核心抽象层次总结

| 层次 | 抽象 | 职责 | 关键类 |
|------|------|------|--------|
| **L1: 用户接口** | 协程 API | 提供用户友好的协程创建/同步接口 | `Fiber`, `FiberLatch`, `FiberMutex` |
| **L2: 调度抽象** | 调度组 | 管理 Worker 线程和调度器 | `SchedulingGroup`, `Scheduling` |
| **L3: 执行引擎** | Fiber 实体 | 协程控制块和上下文切换 | `FiberEntity`, `jump_context` |
| **L4: 内存管理** | 栈分配 | 栈内存分配和保护 | `StackAllocator`, `MemoryPool` |
| **L5: 硬件抽象** | 寄存器 | CPU 寄存器保存/恢复 | `callee-saved registers` |

## 十二、源码目录结构

```
trpc/
├── runtime/threadmodel/fiber/          # Fiber 运行时
│   ├── fiber_thread_model.h/cc         # 线程模型入口
│   └── detail/
│       ├── scheduling_group.h/cc       # 调度组 核心
│       ├── fiber_worker.h/cc           # Worker 线程
│       ├── timer_worker.h/cc           # 定时器线程
│       ├── fiber_entity.h/cc           # Fiber 控制块
│       ├── fiber_desc.h/cc             # Fiber 描述符
│       ├── stack_allocator_impl.h/cc   # 栈分配器
│       ├── waitable.h/cc               # 等待链
│       ├── scheduling/
│       │   ├── scheduling.h/cc         # 调度器接口
│       │   ├── v1/scheduling_impl.h/cc # V1: 全局队列
│       │   └── v2/scheduling_impl.h/cc # V2: 本地队列
│       └── fcontext/
│           ├── x86_64/jump_context.S   # x86_64 上下文切换
│           └── aarch64/jump_context.S  # aarch64 上下文切换
└── coroutine/                          # 用户 API
    ├── fiber.h/cc                      # Fiber 类
    ├── fiber_latch.h/cc                # FiberLatch
    ├── fiber_condition_variable.h/cc   # FiberConditionVariable
    ├── fiber_event.h                   # FiberEvent
    ├── fiber_mutex.h                   # FiberMutex
    ├── fiber_local.h                   # FiberLocal
    └── future.h                        # Fiber Future
```
