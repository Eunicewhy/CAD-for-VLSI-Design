import matplotlib.pyplot as plt
import matplotlib.patches as patches
import random
import sys

def visualize(output_file):
    with open(output_file) as f:
        lines = f.readlines()
    area = float(lines[0])
    width, height = map(float, lines[1].split())
    inl = float(lines[2])

    fig, ax = plt.subplots()
    ax.set_xlim(0, width + 10)
    ax.set_ylim(0, height + 10)
    ax.set_aspect('equal')
    ax.set_title(f"Area={area:.2f}, INL={inl:.2f}")

    for line in lines[3:]:
        tokens = line.strip().split()
        name = tokens[0]
        x, y = float(tokens[1]), float(tokens[2])
        w, h = float(tokens[3][1:]), float(tokens[4])
        color = (random.random(), random.random(), random.random())
        rect = patches.Rectangle((x, y), w, h, linewidth=1, edgecolor='black', facecolor=color, alpha=0.5)
        ax.add_patch(rect)
        ax.text(x + w/2, y + h/2, name, ha='center', va='center', fontsize=8)

    ax.add_patch(patches.Rectangle((0, 0), width, height, fill=False, linestyle='--', linewidth=2, edgecolor='red'))
    plt.grid(True)
    plt.show()

if __name__ == "__main__":
    visualize(sys.argv[1])  
