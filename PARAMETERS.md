# Tối ưu tham số cho các thuật toán

Tài liệu này mô tả các tham số đã được tối ưu cho từng thuật toán, với khả năng tự động điều chỉnh dựa trên kích thước bài toán.

## Genetic Algorithm (GA)

### Tham số hiện tại (Adaptive)
- **Population Size**: `max(30, min(100, dimension * 2))`
  - Tối thiểu: 30
  - Tối đa: 100
  - Tỷ lệ: 2x dimension
  - Lý do: Quần thể lớn hơn cho bài toán lớn, nhưng giới hạn để tránh overhead

- **Max Generations**: `max(300, min(800, dimension * 15))`
  - Tối thiểu: 300
  - Tối đa: 800
  - Tỷ lệ: 15x dimension
  - Lý do: Bài toán lớn cần nhiều thế hệ hơn để hội tụ

- **Patience**: `max(100, min(200, MAX_GENERATIONS / 3))`
  - Tối thiểu: 100
  - Tối đa: 200
  - Tỷ lệ: 1/3 của max generations
  - Lý do: Early stopping khi không cải thiện

### Lý do tối ưu
- Population size vừa đủ để duy trì diversity nhưng không quá lớn
- Generations đủ để hội tụ nhưng có early stopping
- Cân bằng giữa exploration và exploitation

---

## Evolution Strategies (ES)

### Tham số hiện tại (Adaptive)
- **μ (Mu - Parents)**: `max(20, min(50, dimension * 1.5))`
  - Tối thiểu: 20
  - Tối đa: 50
  - Tỷ lệ: 1.5x dimension

- **λ (Lambda - Offspring)**: `μ * 4`
  - Tỷ lệ: 4:1 (offspring:parents)
  - Lý do: Tỷ lệ chuẩn trong ES literature

- **Max Generations**: `max(250, min(600, dimension * 12))`
  - Tối thiểu: 250
  - Tối đa: 600

- **Patience**: `max(80, min(150, MAX_GENERATIONS / 3))`
  - Tối thiểu: 80
  - Tối đa: 150

- **Selection**: `(μ + λ)` - Plus selection
  - Lý do: Giữ lại parents tốt, tốt hơn (μ, λ) cho CVRP

### Lý do tối ưu
- Tỷ lệ μ:λ = 1:4 là chuẩn trong ES
- Plus selection tốt hơn comma selection cho bài toán này
- Adaptive theo dimension để cân bằng exploration/exploitation

---

## Evolutionary Programming (EP)

### Tham số hiện tại (Adaptive)
- **Population Size**: `max(30, min(80, dimension * 1.8))`
  - Tối thiểu: 30
  - Tối đa: 80
  - Tỷ lệ: 1.8x dimension
  - Lý do: Nhỏ hơn GA một chút vì EP chỉ dùng mutation

- **Max Generations**: `max(300, min(700, dimension * 14))`
  - Tối thiểu: 300
  - Tối đa: 700

- **Patience**: `max(100, min(180, MAX_GENERATIONS / 3))`

- **Tournament Size**: `3` (fixed)
  - Lý do: Tournament size 3 là chuẩn, cân bằng selection pressure

### Lý do tối ưu
- Population nhỏ hơn GA vì không có crossover
- Tournament selection với size 3 là optimal
- Adaptive parameters tương tự GA

---

## Ant Colony Optimization (ACO)

### Tham số hiện tại (Adaptive)
- **Number of Ants**: `max(25, min(50, dimension * 1.2))`
  - Tối thiểu: 25
  - Tối đa: 50
  - Tỷ lệ: 1.2x dimension

- **Max Iterations**: `max(150, min(400, dimension * 8))`
  - Tối thiểu: 150
  - Tối đa: 400

- **Patience**: `max(40, min(80, MAX_ITERATIONS / 4))`

### Tham số pheromone (Fixed, optimized)
- **α (Alpha)**: `1.0` - Pheromone importance
  - Lý do: Giá trị chuẩn, cân bằng với heuristic

- **β (Beta)**: `3.0` - Heuristic importance
  - **Tăng từ 2.0 lên 3.0**: Tăng cường exploitation của heuristic (1/distance)
  - Lý do: Heuristic tốt cho CVRP, nên ưu tiên hơn

- **ρ (Rho)**: `0.05` - Evaporation rate
  - **Giảm từ 0.1 xuống 0.05**: Evaporation chậm hơn
  - Lý do: Giữ lại thông tin pheromone lâu hơn, tốt cho exploration

- **Q**: `100.0` - Pheromone deposit constant
  - Lý do: Giá trị chuẩn, điều chỉnh lượng pheromone deposit

- **τ₀ (Tau0)**: `1.0 / (dimension * nn_estimate)`
  - Adaptive dựa trên dimension
  - Lý do: Initial pheromone nên tỷ lệ nghịch với problem size

### Lý do tối ưu
- **β tăng**: Heuristic (khoảng cách) rất quan trọng cho CVRP
- **ρ giảm**: Giữ lại thông tin tốt lâu hơn, tránh premature convergence
- Số ants vừa đủ để explore nhưng không quá tốn thời gian

---

## So sánh tham số cũ vs mới

### GA
| Tham số | Cũ | Mới | Cải thiện |
|---------|----|----|-----------|
| Population | 50 (fixed) | 30-100 (adaptive) | Tự động scale |
| Max Gen | 500 (fixed) | 300-800 (adaptive) | Linh hoạt hơn |
| Patience | 150 (fixed) | 100-200 (adaptive) | Tối ưu theo gen |

### ES
| Tham số | Cũ | Mới | Cải thiện |
|---------|----|----|-----------|
| μ | 30 (fixed) | 20-50 (adaptive) | Tự động scale |
| λ | 120 (fixed) | 80-200 (adaptive) | Tỷ lệ 4:1 được giữ |
| Max Gen | 400 (fixed) | 250-600 (adaptive) | Linh hoạt hơn |

### EP
| Tham số | Cũ | Mới | Cải thiện |
|---------|----|----|-----------|
| Population | 50 (fixed) | 30-80 (adaptive) | Nhỏ hơn GA (hợp lý) |
| Max Gen | 500 (fixed) | 300-700 (adaptive) | Tương tự GA |

### ACO
| Tham số | Cũ | Mới | Cải thiện |
|---------|----|----|-----------|
| Ants | 30 (fixed) | 25-50 (adaptive) | Tự động scale |
| Max Iter | 200 (fixed) | 150-400 (adaptive) | Linh hoạt hơn |
| β | 2.0 | **3.0** | **Tăng exploitation** |
| ρ | 0.1 | **0.05** | **Tăng exploration** |

---

## Kết quả mong đợi

### Cải thiện
1. **Adaptive scaling**: Tham số tự động điều chỉnh theo kích thước bài toán
2. **Better exploration**: ACO với ρ thấp hơn giữ lại thông tin tốt lâu hơn
3. **Better exploitation**: ACO với β cao hơn ưu tiên heuristic tốt
4. **Balanced runtime**: Tham số được tối ưu để cân bằng chất lượng và thời gian

### Trade-offs
- **Bài toán nhỏ (< 40 nodes)**: Tham số nhỏ hơn → nhanh hơn
- **Bài toán lớn (> 60 nodes)**: Tham số lớn hơn → tốt hơn nhưng chậm hơn
- **Early stopping**: Tất cả thuật toán đều có patience để tránh lãng phí thời gian

---

## Ghi chú

- Tất cả tham số được tính toán tự động dựa trên `cvrp.dimension`
- Có thể điều chỉnh thủ công trong `main.cpp` nếu cần
- Các tham số pheromone của ACO (α, β, ρ) là fixed nhưng đã được tối ưu dựa trên literature
- Early stopping (patience) giúp tiết kiệm thời gian khi không cải thiện

