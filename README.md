# Evolutionary Image Reconstruction in C++

This project provides a real-time visualization of evolutionary algorithms, specifically a **Genetic Algorithm** and a **Hill Climbing Algorithm**, as they work to reconstruct a target image from scratch. The entire process is rendered using a custom-built, hardware-accelerated 2D engine using Direct2D on Windows.

![Showcase GIF](./GA&HC.gif)

---

## 📜 Overview

The core idea is to treat an image as an "individual" in a population, where its "genes" are the pixel data. The algorithms start with a population of randomly generated images and iteratively "evolve" them over generations to more closely match a target image. The "fitness" of an individual is determined by how similar it is to the target.

This application runs both a Genetic Algorithm and a Hill Climbing algorithm side-by-side in three different modes, allowing for a direct visual comparison of their performance and behavior:
* **Full Color:** Evolves the full RGBA value for each pixel.
* **Grayscale:** Evolves pixels where R, G, and B values are identical.
* **Binary (Black & White):** Evolves pixels that can only be pure black or pure white.

## ✨ Features

- **Dual Algorithm Implementation:** Compare a **Genetic Algorithm** and a **Hill Climbing** algorithm in real-time.
- **Multi-threaded Execution:** Each algorithm and color mode runs in its own dedicated thread for maximum performance.
- **Real-time Visualization:** A custom rendering engine built with **Direct2D** and **DirectWrite** displays the progress of each algorithm, including the current best specimen, its fitness score, and the current generation number.
- **Three Evolution Modes:** Watch the image evolve in **Full Color**, **Grayscale**, and **Binary**.
- **High-Performance C++:** The project is written in modern C++ for efficiency.

---

## ⚙️ How It Works

### Fitness Calculation
The fitness of a given image (an "individual") is a score from 0.0 to 1.0 that indicates how closely it matches the target image. It is calculated based on the sum of the absolute differences between the pixel values of the generated image and the target image. A fitness of `1.0` represents a perfect match.

### Genetic Algorithm
1.  **Initialization:** A population of random individuals (images) is created.
2.  **Selection:** The best-performing individuals from the current population are selected to be "parents" for the next generation. This implementation prioritizes individuals with higher fitness scores.
3.  **Crossover:** Two parent individuals are combined to create a new "offspring". A random crossover point is chosen, and the new individual's genome is created by taking the first part from one parent and the remaining part from the other.
4.  **Mutation:** To introduce new genetic material and avoid local maxima, each gene (pixel component) in the offspring's genome has a small random chance to be changed to a new random value.
5.  **Repeat:** The new population replaces the old, and the cycle repeats.

### Hill Climbing
1.  **Initialization:** A single random individual is created.
2.  **Neighborhood:** A "neighbor" is created by making a small mutation to the current individual.
3.  **Selection:** The fitness of the neighbor is calculated. If the neighbor is fitter than the current individual, it replaces the current individual.
4.  **Restart:** If the algorithm fails to find a better neighbor for a large number of attempts (defined by `failure_threshold`), it is considered stuck in a local maximum and restarts with a new random individual.
5.  **Repeat:** The process continues until a high fitness score is achieved.

---

## 🚀 Getting Started

### Prerequisites
- Windows 10 or later
- Visual Studio with the "Desktop development with C++" workload
- Windows SDK

### Building and Running
1.  Clone the repository:
    ```bash
    git clone [https://github.com/TripleSix6660/Genetic-Hill-Climbing.git)
    ```
2.  Open the solution file (`.sln`) in Visual Studio.
3.  Set the build configuration to **Release** and the architecture to **x64**.
4.  Build the solution (F7 or `Build > Build Solution`).
5.  Run the executable from the output directory or directly from Visual Studio (F5).
6.  Press the **`END`** key to close the application.

### Configuration
You can tweak the parameters of the algorithms directly in the source code.

#### Algorithm Parameters:
- **Genetic Algorithm settings** are in `genetic/genetic.h`.
- **Hill Climbing settings** are in `hill_climbing/hill_climbing.h`.

Key parameters include:
- `population_size`: The number of individuals in the genetic algorithm's population.
- `mutation_rate`: The probability of a gene mutating.
- `failure_threshold`: The number of attempts before the hill-climber restarts.
- `interval`: The number of generations between performance pauses to prevent system overload.

#### Changing the Target Image:
The target image is hard-coded as a byte array within the project. To change it, you must modify the `globals.h` file:

1.  **Get Image Byte Code:** You need to convert your desired image into a C++ byte array. The data must be in **32-bit BGRA format** (Blue, Green, Red, Alpha). You can use various online tools or scripts to do this.
2.  **Update `globals.h`:**
    -   Replace the contents of the `image_byte_code_array` with your new byte code.
    -   Update the `image_width` and `image_height` constant values to match the dimensions of your new image.

