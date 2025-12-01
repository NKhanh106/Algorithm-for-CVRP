# Áp dụng giải thuật tiến hóa cho CVRP

> 📖 **Xem báo cáo chi tiết**: [REPORT.md](REPORT.md) - Báo cáo đầy đủ về bài toán, phương pháp, kết quả và so sánh

Dự án này triển khai quy trình giải bài toán Vehicle Routing Problem với ràng buộc tải trọng (CVRP) theo hai giai đoạn:
- **Xây dựng lời giải khởi tạo** bằng thuật toán Clark & Wright (thư mục `constructive/`), kèm tối ưu cục bộ 2-opt/3-opt.
- **Cải thiện lời giải** bằng các metaheuristic: **GA** (Genetic Algorithm), **ES** (Evolution Strategies), **EP** (Evolutionary Programming), và **ACO** (Ant Colony Optimization), sử dụng không gian lời giải xuất phát từ Clark & Wright và đánh giá tuyến bằng quy hoạch động.

## Các bước đã hoàn thành
- ✅ Chuẩn hóa cấu trúc mã nguồn: tách riêng Clark & Wright, GA, ES, EP, ACO để dễ mở rộng.
- ✅ Triển khai đầy đủ 4 metaheuristic: **GA**, **ES**, **EP**, và **ACO**.
- ✅ Tự động đọc toàn bộ bộ dữ liệu trong `Vrp-Set-A-input`, chạy cả 4 thuật toán và so sánh kết quả.
- ✅ Ghi kết quả chi tiết vào `Vrp-Set-A-output` với format có banner cho từng phương pháp.
- ✅ Hiển thị so sánh kết quả trong console và ghi optimal value để đánh giá.
- ✅ Script Python để vẽ biểu đồ so sánh hiệu suất (xem `README_visualization.md`).

## Cấu trúc thư mục chính
- `constructive/`: Thuật toán Clark & Wright + tối ưu 2-opt/3-opt.
- `ga/`: Lõi GA, toán tử lai ghép/đột biến, các hàm đánh giá tuyến.
- `es/`: Evolution Strategies với (μ + λ) selection và adaptive mutation.
- `ep/`: Evolutionary Programming với tournament selection và multiple mutation operators.
- `aco/`: Ant Colony Optimization với pheromone-based construction, 2-opt local search, và elite-based pheromone update.
- `Vrp-Set-A-input/`: Dữ liệu chuẩn hóa từ bộ A (file `.vrp`).
- `Vrp-Set-A-output/`: Lời giải của các thuật toán (file `.sol`).
- `plots/`: Thư mục chứa biểu đồ so sánh (tạo bởi `visualize_results.py`).

## Hướng dẫn chạy nhanh

### 1. Biên dịch chương trình
```bash
g++ -std=c++17 -O2 main.cpp CVRP.cpp \
    ga/GA_core.cpp ga/GA_Operators.cpp \
    es/ES_Core.cpp ep/EP_Core.cpp aco/ACO_Core.cpp \
    constructive/ClarkWright.cpp -o CVRP_run.exe
```

### 2. Chạy các thuật toán
```bash
./CVRP_run.exe
```

Chương trình sẽ:
- Tuần tự xử lý mọi file `.vrp` trong `Vrp-Set-A-input`
- Chạy **GA**, **ES**, **EP**, và **ACO** cho từng bài toán
- Hiển thị so sánh kết quả 4 thuật toán trong console
- Ghi kết quả chi tiết vào `Vrp-Set-A-output` (file `.sol`)

### 3. Vẽ biểu đồ so sánh (tùy chọn)
```bash
# Cài đặt dependencies
pip install matplotlib pandas numpy

# Chạy script visualization
python visualize_results.py
```

Script sẽ tạo các biểu đồ so sánh trong folder `plots/`:
- Fitness comparison
- Gap from optimal
- Execution time
- Time vs Fitness trade-off
- Heatmap và boxplot
- Summary statistics (CSV)

Xem chi tiết trong `README_visualization.md`.

## Format file output

Mỗi file `.sol` chứa:
- Thông tin chung: Name, Dimension, Trucks, Capacity, Optimal
- Kết quả từng thuật toán với banner:
  - `==============GA===============`
  - `==============ES===============`
  - `==============EP===============`
  - `==============ACO===============`
- Mỗi section gồm: Best Fitness, Vehicles Used, Generations, Time, Routes

## Tài Liệu Tham Khảo

- **[REPORT.md](REPORT.md)**: Báo cáo chi tiết về bài toán, phương pháp, kết quả và so sánh
- **[PARAMETERS.md](PARAMETERS.md)**: Tài liệu về các tham số đã tối ưu
- **[README_visualization.md](README_visualization.md)**: Hướng dẫn vẽ biểu đồ so sánh

## Công việc cần làm thêm
- ⏳ Xây dựng cơ chế cấu hình để chọn thuật toán tiến hóa khi chạy (chạy riêng lẻ hoặc kết hợp).
- ⏳ Tối ưu hóa các thuật toán hiện tại (thêm 3-opt cho ACO, cải thiện pheromone update strategies).
- ⏳ Thêm logging chi tiết hơn (convergence curves, diversity metrics, pheromone visualization).
- ⏳ Thử nghiệm với các tham số khác nhau và tự động tuning.