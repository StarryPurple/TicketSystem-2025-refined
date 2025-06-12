import asyncio
import websockets
import subprocess
import os

# 获取当前脚本的目录 (即 bridge 文件夹)
bridge_dir = os.path.dirname(os.path.abspath(__file__))
# C++项目根目录 (bridge 文件夹的上一级)
project_root = os.path.abspath(os.path.join(bridge_dir, os.pardir))

# C++可执行文件的路径，现在明确指向 build/code
cpp_executable_path = os.path.join(project_root, 'build', 'code')

async def handle_websocket(websocket, path=None): # 保持 path=None 的修改
    print(f"Client connected from {websocket.remote_address}")

    process = None
    try:
        # 启动C++子进程
        # universal_newlines=False，意味着 stdin/stdout/stderr 是字节流
        process = await asyncio.create_subprocess_exec(
            cpp_executable_path,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            bufsize=0, # 修改这里：将 bufsize 设置为 0 (无缓冲) 或 -1 (系统默认)
            # 对于字节流，通常设置为 0 效果最好，可以实时读写
            universal_newlines=False, # <-- 关键修改：设置为 False
            cwd=project_root # 将工作目录设置为C++项目根目录
        )
        print(f"C++ process '{cpp_executable_path}' started.")

        async for message in websocket:
            print(f"Received from client: {message}")
            try:
                # 将前端消息（字符串）编码为字节串，并加上换行符
                # 默认使用 UTF-8 编码
                process.stdin.write((message + '\n').encode('utf-8'))
                await process.stdin.drain() # 确保数据被发送

                # 从C++程序的标准输出读取一行字节串
                # readline() 读取到换行符或 EOF
                output_line_bytes = await process.stdout.readline()
                # 将字节串解码为字符串，并移除末尾的换行符
                output_line = output_line_bytes.decode('utf-8').strip()

                print(f"Sent to client: {output_line}")
                await websocket.send(output_line)

            except Exception as e:
                print(f"Error communicating with C++ process: {e}")
                await websocket.send(f"Error: {e}")
                break # 退出循环，关闭连接

    except FileNotFoundError:
        error_msg = f"Error: C++ executable not found at {cpp_executable_path}. Please check the path and if it's compiled."
        print(error_msg)
        await websocket.send(error_msg)
    except Exception as e:
        print(f"Error starting C++ process or handling websocket: {e}")
        await websocket.send(f"Server error: {e}")
    finally:
        if process and process.returncode is None:
            print("Terminating C++ process...")
            process.terminate()
            await process.wait() # 等待进程结束
        print(f"Client disconnected from {websocket.remote_address}")

async def main():
    # WebSocket服务器将在 8765 端口监听
    server = await websockets.serve(handle_websocket, "localhost", 8765)
    print("WebSocket server started on ws://localhost:8765")
    await server.wait_closed()

if __name__ == "__main__":
    asyncio.run(main())