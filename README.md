# Áp dụng giải thuật tiến hóa cho CVRP

Dự án này triển khai quy trình giải bài toán Vehicle Routing Problem với ràng buộc tải trọng (CVRP) theo hai giai đoạn:
- **Xây dựng lời giải khởi tạo** bằng thuật toán Clark & Wright (thư mục `constructive/`), kèm tối ưu cục bộ 2-opt/3-opt.
- **Cải thiện lời giải** bằng các metaheuristic trong thư mục `ga/` (Genetic Algorithm) và `es/` (Evolution Strategies), sử dụng không gian lời giải xuất phát từ Clark & Wright và đánh giá tuyến bằng quy hoạch động.

## Các bước đã hoàn thành
- Chuẩn hóa cấu trúc mã nguồn: tách riêng Clark & Wright, GA, ES để dễ mở rộng.
- Tự động đọc toàn bộ bộ dữ liệu trong `Vrp-Set-A-input`, lần lượt chạy GA và ES rồi ghi kết quả sang `Vrp-Set-A-output`.
- Cung cấp thông số thời gian chạy, số thế hệ, fitness tốt nhất của từng metaheuristic vào file `.sol`.
- Chuẩn hóa build: 
  ```
  g++ -std=c++17 -O2 main.cpp CVRP.cpp \
      ga/GA_core.cpp ga/GA_Operators.cpp \
      es/ES_Core.cpp \
      constructive/ClarkWright.cpp -o CVRP_run.exe
  ```

## Cấu trúc thư mục chính
- `constructive/`: Thuật toán Clark & Wright + tối ưu 2-opt/3-opt.
- `ga/`: Lõi GA, toán tử lai ghép/đột biến, các hàm đánh giá tuyến.
- `EP/`, `ES/`, `ACO/`: thư mục trống để chuẩn bị tích hợp Evolutionary Programming, Evolution Strategies và Ant Colony Optimization.
- `Vrp-Set-A-input/`: dữ liệu chuẩn hóa từ bộ A.
- `Vrp-Set-A-output/`: lời giải GA sinh ra sau mỗi lần chạy.
- `CVRP_run.exe`: file chạy sau khi biên dịch.

## Hướng dẫn chạy nhanh
```bash
g++ -std=c++17 -O2 main.cpp CVRP.cpp \
    ga/GA_core.cpp ga/GA_Operators.cpp \
    es/ES_Core.cpp \
    constructive/ClarkWright.cpp -o CVRP_run.exe
./CVRP_run.exe
```
Chương trình sẽ tuần tự xử lý mọi file `.vrp` trong `Vrp-Set-A-input`, chạy GA rồi ES cho từng bài toán và sinh kết quả tương ứng trong `Vrp-Set-A-output`.

## Công việc cần làm thêm
- Bổ sung hiện thực **EP** và **ACO** trong các thư mục tương ứng và tái sử dụng không gian lời giải từ Clark & Wright.
- Xây dựng cơ chế cấu hình để chọn thuật toán tiến hóa khi chạy.
- Thêm thống kê/đồ thị để so sánh hiệu năng các metaheuristic.