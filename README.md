# OS_ZXY

## 环境要求

- Linux平台

- make

- cmake 

## 构建命令

```bash
mkdir build

cd build

cmake ..

make 
```

## 运行方法

```bash
cd bin

./hello
```

## 测试

测试文件在`src/just_test.cpp`, 如果想要进行测试, 就在其中的`func()`函数中添加要执行的代码即可, 如果想要新增文件, 在CMakeLists.txt中添加源文件(注意格式), 其他构建方法同上


## 创建并虚拟化镜像文件为虚拟块设备

- 赋予脚本可执行权限

    ```bash
    chmod +x generate_img.sh
    ```

- 运行脚本

    ```bash
    ./generate_img.sh 
    ```

此时可以生成一个空白的`disk.img`文件

如果要删除loop, 需要执行:

```bash
losetup -a          # 检查生成的.img的loop号

sudo losetup -d /dev/loopN      # 如果号为loopN, 则删除记录
```

## 使用spdlog进行日志打印

详情见https://github.com/gabime/spdlog.git


Third-party components:

This project includes code from [spdlog](https://github.com/gabime/spdlog), 
which is licensed under the MIT License.

Copyright (c) 2016 Gabi Melman
See third_party/spdlog/LICENSE.txt for full license text.




