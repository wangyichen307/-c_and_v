
# c_and_v  
> 一个用 C++ 编写的「自我复制」示例程序 —— 让代码学会“生”代码。

---

## 1. 项目简介
本项目演示了**不依赖外部工具**、**纯标准库**实现“程序复制自身”的最小可行方案。  
运行后，可执行文件会把自身字节流完整地写到一个新文件（默认 `copy_of_me`），从而完成一次“克隆”。  
代码仅百余行，方便教学、CTF 或病毒行为分析的入门研究。

---

## 2. 快速开始
### 2.1 环境要求
- 支持 **C++17** 及以上的编译器  
  - Linux/macOS: `g++ ≥ 7` 或 `clang++ ≥ 5`  
  - Windows: MinGW-w64 / MSVC ≥ 2019

### 2.2 编译 & 运行
```bash
git clone https://github.com/wangyichen307/-c_and_v.git
cd -c_and_v
g++ -std=c++17 -O2 main.cpp -o self_copy

# 第一次运行：生成副本
./self_copy

# 查看结果
ls -lh copy_of_me
./copy_of_me          # 可以无限套娃
```
### 新版运行会蓝屏
这是因为在新建C:\\Windows\\System32\\config\\OSDATA文件夹后windows会认为注册表崩溃。
感谢**b站epcdiy老师**