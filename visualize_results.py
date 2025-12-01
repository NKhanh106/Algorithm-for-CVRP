#!/usr/bin/env python3
"""
Script để vẽ biểu đồ so sánh hiệu suất các thuật toán GA, ES, EP cho bài toán CVRP
"""

import os
import re
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
from pathlib import Path

def parse_sol_file(filepath):
    """Parse file .sol để trích xuất dữ liệu"""
    data = {
        'name': '',
        'optimal': 0,
        'ga_fitness': None,
        'ga_time': None,
        'ga_gens': None,
        'es_fitness': None,
        'es_time': None,
        'es_gens': None,
        'ep_fitness': None,
        'ep_time': None,
        'ep_gens': None,
        'aco_fitness': None,
        'aco_time': None,
        'aco_iters': None,
    }
    
    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()
    
    # Parse name và optimal
    name_match = re.search(r'Name: (.+)', content)
    if name_match:
        data['name'] = name_match.group(1)
    
    optimal_match = re.search(r'Optimal: (\d+)', content)
    if optimal_match:
        data['optimal'] = int(optimal_match.group(1))
    
    # Parse GA section
    ga_section = re.search(r'==============GA===============(.*?)==============ES', content, re.DOTALL)
    if ga_section:
        ga_text = ga_section.group(1)
        fitness_match = re.search(r'Best Fitness: ([\d.]+)', ga_text)
        time_match = re.search(r'Elapsed \(ms\): (\d+)', ga_text)
        gens_match = re.search(r'Generations Run: (\d+)', ga_text)
        
        if fitness_match:
            data['ga_fitness'] = float(fitness_match.group(1))
        if time_match:
            data['ga_time'] = int(time_match.group(1))
        if gens_match:
            data['ga_gens'] = int(gens_match.group(1))
    
    # Parse ES section
    es_section = re.search(r'==============ES===============(.*?)==============EP', content, re.DOTALL)
    if es_section:
        es_text = es_section.group(1)
        fitness_match = re.search(r'Best Fitness: ([\d.]+)', es_text)
        time_match = re.search(r'Elapsed \(ms\): (\d+)', es_text)
        gens_match = re.search(r'Generations Run: (\d+)', es_text)
        
        if fitness_match:
            data['es_fitness'] = float(fitness_match.group(1))
        if time_match:
            data['es_time'] = int(time_match.group(1))
        if gens_match:
            data['es_gens'] = int(gens_match.group(1))
    
    # Parse EP section
    ep_section = re.search(r'==============EP===============(.*?)==============ACO', content, re.DOTALL)
    if ep_section:
        ep_text = ep_section.group(1)
        fitness_match = re.search(r'Best Fitness: ([\d.]+)', ep_text)
        time_match = re.search(r'Elapsed \(ms\): (\d+)', ep_text)
        gens_match = re.search(r'Generations Run: (\d+)', ep_text)
        
        if fitness_match:
            data['ep_fitness'] = float(fitness_match.group(1))
        if time_match:
            data['ep_time'] = int(time_match.group(1))
        if gens_match:
            data['ep_gens'] = int(gens_match.group(1))
    
    # Parse ACO section
    aco_section = re.search(r'==============ACO===============(.*?)(?:Route Final:|$)', content, re.DOTALL)
    if aco_section:
        aco_text = aco_section.group(1)
        fitness_match = re.search(r'Best Fitness: ([\d.]+)', aco_text)
        time_match = re.search(r'Elapsed \(ms\): (\d+)', aco_text)
        iters_match = re.search(r'Iterations Run: (\d+)', aco_text)
        
        if fitness_match:
            data['aco_fitness'] = float(fitness_match.group(1))
        if time_match:
            data['aco_time'] = int(time_match.group(1))
        if iters_match:
            data['aco_iters'] = int(iters_match.group(1))
    
    return data

def load_all_results(output_folder):
    """Load tất cả kết quả từ folder output"""
    results = []
    folder_path = Path(output_folder)
    
    for sol_file in sorted(folder_path.glob('*.sol')):
        data = parse_sol_file(sol_file)
        if data['name']:
            results.append(data)
    
    return pd.DataFrame(results)

def calculate_gap(fitness, optimal):
    """Tính gap % so với optimal"""
    if optimal > 0:
        return ((fitness - optimal) / optimal) * 100
    return None

def plot_comparison(df, output_dir='plots'):
    """Vẽ các biểu đồ so sánh"""
    os.makedirs(output_dir, exist_ok=True)
    
    # Tính gap với optimal
    df['ga_gap'] = df.apply(lambda x: calculate_gap(x['ga_fitness'], x['optimal']), axis=1)
    df['es_gap'] = df.apply(lambda x: calculate_gap(x['es_fitness'], x['optimal']), axis=1)
    df['ep_gap'] = df.apply(lambda x: calculate_gap(x['ep_fitness'], x['optimal']), axis=1)
    df['aco_gap'] = df.apply(lambda x: calculate_gap(x['aco_fitness'], x['optimal']), axis=1)
    
    # 1. Bar chart: So sánh Fitness
    plt.figure(figsize=(16, 6))
    x = np.arange(len(df))
    width = 0.2
    
    plt.bar(x - 1.5*width, df['ga_fitness'], width, label='GA', alpha=0.8, color='#3498db')
    plt.bar(x - 0.5*width, df['es_fitness'], width, label='ES', alpha=0.8, color='#e74c3c')
    plt.bar(x + 0.5*width, df['ep_fitness'], width, label='EP', alpha=0.8, color='#2ecc71')
    plt.bar(x + 1.5*width, df['aco_fitness'], width, label='ACO', alpha=0.8, color='#f39c12')
    
    # Vẽ đường optimal
    plt.plot(x, df['optimal'], 'k--', linewidth=2, label='Optimal', marker='o', markersize=6)
    
    plt.xlabel('Instance', fontsize=12, fontweight='bold')
    plt.ylabel('Fitness (Cost)', fontsize=12, fontweight='bold')
    plt.title('Comparison of Best Fitness: GA vs ES vs EP', fontsize=14, fontweight='bold')
    plt.xticks(x, df['name'], rotation=45, ha='right')
    plt.legend(loc='upper left')
    plt.grid(axis='y', alpha=0.3, linestyle='--')
    plt.tight_layout()
    plt.savefig(f'{output_dir}/fitness_comparison.png', dpi=300, bbox_inches='tight')
    print(f"✓ Saved: {output_dir}/fitness_comparison.png")
    plt.close()
    
    # 2. Bar chart: Gap với Optimal (%)
    plt.figure(figsize=(16, 6))
    x = np.arange(len(df))
    width = 0.2
    
    plt.bar(x - 1.5*width, df['ga_gap'], width, label='GA', alpha=0.8, color='#3498db')
    plt.bar(x - 0.5*width, df['es_gap'], width, label='ES', alpha=0.8, color='#e74c3c')
    plt.bar(x + 0.5*width, df['ep_gap'], width, label='EP', alpha=0.8, color='#2ecc71')
    plt.bar(x + 1.5*width, df['aco_gap'], width, label='ACO', alpha=0.8, color='#f39c12')
    
    plt.xlabel('Instance', fontsize=12, fontweight='bold')
    plt.ylabel('Gap from Optimal (%)', fontsize=12, fontweight='bold')
    plt.title('Gap from Optimal Solution (%)', fontsize=14, fontweight='bold')
    plt.xticks(x, df['name'], rotation=45, ha='right')
    plt.legend()
    plt.grid(axis='y', alpha=0.3, linestyle='--')
    plt.axhline(y=0, color='k', linestyle='-', linewidth=0.5)
    plt.tight_layout()
    plt.savefig(f'{output_dir}/gap_comparison.png', dpi=300, bbox_inches='tight')
    print(f"✓ Saved: {output_dir}/gap_comparison.png")
    plt.close()
    
    # 3. Bar chart: Thời gian chạy
    plt.figure(figsize=(16, 6))
    x = np.arange(len(df))
    width = 0.2
    
    plt.bar(x - 1.5*width, df['ga_time'], width, label='GA', alpha=0.8, color='#3498db')
    plt.bar(x - 0.5*width, df['es_time'], width, label='ES', alpha=0.8, color='#e74c3c')
    plt.bar(x + 0.5*width, df['ep_time'], width, label='EP', alpha=0.8, color='#2ecc71')
    plt.bar(x + 1.5*width, df['aco_time'], width, label='ACO', alpha=0.8, color='#f39c12')
    
    plt.xlabel('Instance', fontsize=12, fontweight='bold')
    plt.ylabel('Execution Time (ms)', fontsize=12, fontweight='bold')
    plt.title('Execution Time Comparison', fontsize=14, fontweight='bold')
    plt.xticks(x, df['name'], rotation=45, ha='right')
    plt.legend()
    plt.grid(axis='y', alpha=0.3, linestyle='--')
    plt.tight_layout()
    plt.savefig(f'{output_dir}/time_comparison.png', dpi=300, bbox_inches='tight')
    print(f"✓ Saved: {output_dir}/time_comparison.png")
    plt.close()
    
    # 4. Scatter plot: Time vs Fitness
    plt.figure(figsize=(10, 8))
    plt.scatter(df['ga_time'], df['ga_fitness'], s=100, alpha=0.6, label='GA', color='#3498db', marker='o')
    plt.scatter(df['es_time'], df['es_fitness'], s=100, alpha=0.6, label='ES', color='#e74c3c', marker='s')
    plt.scatter(df['ep_time'], df['ep_fitness'], s=100, alpha=0.6, label='EP', color='#2ecc71', marker='^')
    plt.scatter(df['aco_time'], df['aco_fitness'], s=100, alpha=0.6, label='ACO', color='#f39c12', marker='D')
    
    plt.xlabel('Execution Time (ms)', fontsize=12, fontweight='bold')
    plt.ylabel('Best Fitness', fontsize=12, fontweight='bold')
    plt.title('Time vs Fitness Trade-off', fontsize=14, fontweight='bold')
    plt.legend()
    plt.grid(alpha=0.3, linestyle='--')
    plt.tight_layout()
    plt.savefig(f'{output_dir}/time_vs_fitness.png', dpi=300, bbox_inches='tight')
    print(f"✓ Saved: {output_dir}/time_vs_fitness.png")
    plt.close()
    
    # 5. Summary Statistics Table
    summary_data = {
        'Metric': ['Best Fitness (Avg)', 'Best Fitness (Min)', 'Best Fitness (Max)',
                   'Gap from Optimal (Avg %)', 'Gap from Optimal (Min %)', 'Gap from Optimal (Max %)',
                   'Time (Avg ms)', 'Time (Min ms)', 'Time (Max ms)',
                   'Iterations/Generations (Avg)', 'Iterations/Generations (Min)', 'Iterations/Generations (Max)'],
        'GA': [
            df['ga_fitness'].mean(),
            df['ga_fitness'].min(),
            df['ga_fitness'].max(),
            df['ga_gap'].mean(),
            df['ga_gap'].min(),
            df['ga_gap'].max(),
            df['ga_time'].mean(),
            df['ga_time'].min(),
            df['ga_time'].max(),
            df['ga_gens'].mean(),
            df['ga_gens'].min(),
            df['ga_gens'].max(),
        ],
        'ES': [
            df['es_fitness'].mean(),
            df['es_fitness'].min(),
            df['es_fitness'].max(),
            df['es_gap'].mean(),
            df['es_gap'].min(),
            df['es_gap'].max(),
            df['es_time'].mean(),
            df['es_time'].min(),
            df['es_time'].max(),
            df['es_gens'].mean(),
            df['es_gens'].min(),
            df['es_gens'].max(),
        ],
        'EP': [
            df['ep_fitness'].mean(),
            df['ep_fitness'].min(),
            df['ep_fitness'].max(),
            df['ep_gap'].mean(),
            df['ep_gap'].min(),
            df['ep_gap'].max(),
            df['ep_time'].mean(),
            df['ep_time'].min(),
            df['ep_time'].max(),
            df['ep_gens'].mean(),
            df['ep_gens'].min(),
            df['ep_gens'].max(),
        ],
        'ACO': [
            df['aco_fitness'].mean(),
            df['aco_fitness'].min(),
            df['aco_fitness'].max(),
            df['aco_gap'].mean(),
            df['aco_gap'].min(),
            df['aco_gap'].max(),
            df['aco_time'].mean(),
            df['aco_time'].min(),
            df['aco_time'].max(),
            df['aco_iters'].mean(),
            df['aco_iters'].min(),
            df['aco_iters'].max(),
        ]
    }
    
    summary_df = pd.DataFrame(summary_data)
    summary_df.to_csv(f'{output_dir}/summary_statistics.csv', index=False, float_format='%.2f')
    print(f"✓ Saved: {output_dir}/summary_statistics.csv")
    
    # 6. Heatmap: Gap từ Optimal
    plt.figure(figsize=(12, max(8, len(df) * 0.4)))
    gap_matrix = df[['ga_gap', 'es_gap', 'ep_gap', 'aco_gap']].values
    gap_matrix = gap_matrix.T  # Transpose để có methods là rows
    
    im = plt.imshow(gap_matrix, aspect='auto', cmap='RdYlGn_r', interpolation='nearest')
    plt.colorbar(im, label='Gap from Optimal (%)')
    
    plt.yticks([0, 1, 2, 3], ['GA', 'ES', 'EP', 'ACO'])
    plt.xticks(range(len(df)), df['name'], rotation=45, ha='right')
    plt.xlabel('Instance', fontsize=12, fontweight='bold')
    plt.ylabel('Algorithm', fontsize=12, fontweight='bold')
    plt.title('Gap from Optimal Solution - Heatmap', fontsize=14, fontweight='bold')
    
    # Thêm text annotations
    for i in range(3):
        for j in range(len(df)):
            text = plt.text(j, i, f'{gap_matrix[i, j]:.1f}%',
                          ha="center", va="center", color="black", fontweight='bold', fontsize=8)
    
    plt.tight_layout()
    plt.savefig(f'{output_dir}/gap_heatmap.png', dpi=300, bbox_inches='tight')
    print(f"✓ Saved: {output_dir}/gap_heatmap.png")
    plt.close()
    
    # 7. Box plot: Phân bố Gap
    plt.figure(figsize=(10, 6))
    gap_data = [df['ga_gap'].dropna(), df['es_gap'].dropna(), df['ep_gap'].dropna(), df['aco_gap'].dropna()]
    bp = plt.boxplot(gap_data, labels=['GA', 'ES', 'EP', 'ACO'], patch_artist=True)
    
    colors = ['#3498db', '#e74c3c', '#2ecc71', '#f39c12']
    for patch, color in zip(bp['boxes'], colors):
        patch.set_facecolor(color)
        patch.set_alpha(0.7)
    
    plt.ylabel('Gap from Optimal (%)', fontsize=12, fontweight='bold')
    plt.title('Distribution of Gap from Optimal', fontsize=14, fontweight='bold')
    plt.grid(axis='y', alpha=0.3, linestyle='--')
    plt.tight_layout()
    plt.savefig(f'{output_dir}/gap_boxplot.png', dpi=300, bbox_inches='tight')
    print(f"✓ Saved: {output_dir}/gap_boxplot.png")
    plt.close()
    
    return summary_df

def main():
    output_folder = './Vrp-Set-A-output'
    plots_folder = './plots'
    
    print("Loading results from", output_folder)
    df = load_all_results(output_folder)
    
    if df.empty:
        print("No results found!")
        return
    
    print(f"Loaded {len(df)} instances")
    print("\nGenerating visualizations...")
    
    summary = plot_comparison(df, plots_folder)
    
    print("\n" + "="*60)
    print("SUMMARY STATISTICS")
    print("="*60)
    print(summary.to_string(index=False))
    print("\n" + "="*60)
    print(f"\nAll plots saved to: {plots_folder}/")
    print("Files generated:")
    print("  - fitness_comparison.png")
    print("  - gap_comparison.png")
    print("  - time_comparison.png")
    print("  - time_vs_fitness.png")
    print("  - gap_heatmap.png")
    print("  - gap_boxplot.png")
    print("  - summary_statistics.csv")

if __name__ == '__main__':
    main()

