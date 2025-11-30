# Hướng dẫn vẽ biểu đồ so sánh thuật toán

## Cài đặt

Cài đặt các thư viện Python cần thiết:

```bash
pip install -r requirements_visualization.txt
```

Hoặc cài đặt trực tiếp:

```bash
pip install matplotlib pandas numpy
```

## Sử dụng

Chạy script để tạo các biểu đồ so sánh:

```bash
python visualize_results.py
```

Script sẽ:
1. Đọc tất cả file `.sol` trong folder `Vrp-Set-A-output/`
2. Parse dữ liệu từ các file
3. Tạo các biểu đồ so sánh và lưu vào folder `plots/`

## Các biểu đồ được tạo

### 1. **fitness_comparison.png**
- Bar chart so sánh fitness của 3 thuật toán (GA, ES, EP)
- Đường kẻ đen (dashed) là giá trị optimal
- Giúp so sánh trực quan chất lượng lời giải

### 2. **gap_comparison.png**
- Bar chart so sánh gap (%) so với optimal
- Gap = ((fitness - optimal) / optimal) * 100
- Giúp đánh giá độ gần với lời giải tối ưu

### 3. **time_comparison.png**
- Bar chart so sánh thời gian chạy (ms)
- Giúp so sánh hiệu suất tính toán

### 4. **time_vs_fitness.png**
- Scatter plot: Time vs Fitness
- Giúp phân tích trade-off giữa thời gian và chất lượng
- Thuật toán ở góc dưới bên trái là tốt nhất (nhanh + tốt)

### 5. **gap_heatmap.png**
- Heatmap hiển thị gap của từng thuật toán cho từng instance
- Màu đỏ = gap lớn (kém), màu xanh = gap nhỏ (tốt)
- Giúp nhận diện thuật toán nào tốt cho instance nào

### 6. **gap_boxplot.png**
- Box plot phân bố gap của 3 thuật toán
- Giúp so sánh độ ổn định và phân bố kết quả

### 7. **summary_statistics.csv**
- Bảng thống kê tổng hợp (mean, min, max)
- Có thể mở bằng Excel hoặc text editor

## Cách đọc biểu đồ

### Fitness Comparison
- **Thấp hơn = tốt hơn** (minimize cost)
- So sánh với đường optimal (đen, dashed)
- Thuật toán nào có cột thấp hơn và gần optimal hơn = tốt hơn

### Gap Comparison
- **Thấp hơn = tốt hơn** (gần optimal hơn)
- Gap = 0% nghĩa là đạt optimal
- Gap < 5% = rất tốt, Gap < 10% = tốt

### Time Comparison
- **Thấp hơn = nhanh hơn**
- Cần cân nhắc trade-off với chất lượng

### Time vs Fitness
- **Góc dưới bên trái = tốt nhất** (nhanh + tốt)
- Góc trên bên phải = kém nhất (chậm + kém)

## Tùy chỉnh

Bạn có thể chỉnh sửa script `visualize_results.py` để:
- Thay đổi kích thước biểu đồ
- Thay đổi màu sắc
- Thêm metrics khác
- Thay đổi format output

## Lưu ý

- Đảm bảo đã chạy các thuật toán và có file `.sol` trong `Vrp-Set-A-output/`
- Script tự động tạo folder `plots/` nếu chưa có
- Tất cả biểu đồ được lưu với độ phân giải cao (300 DPI)

