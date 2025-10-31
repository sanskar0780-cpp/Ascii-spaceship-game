# 🚀 ASCII Spaceship Game

A classic **ASCII-based space shooter** game written in **C++** for the Windows console.  
Pilot your spaceship, destroy enemies, battle bosses, and earn the highest score — all rendered entirely with ASCII characters.

---

## 🧩 Table of Contents
1. [Introduction](#introduction)
2. [Features](#features)
3. [Installation](#installation)
4. [Usage](#usage)
5. [Controls](#controls)
6. [Game Mechanics](#game-mechanics)
7. [Dependencies](#dependencies)
8. [Configuration](#configuration)
9. [Troubleshooting](#troubleshooting)
10. [Future Enhancements](#future-enhancements)
11. [License](#license)

---

## 🕹️ Introduction

**ASCII Spaceship Game** is a console-based space shooter built entirely with ASCII art.  
You control a small spaceship using keyboard or gamepad inputs, dodge enemy fire, shoot incoming enemies, and face off against a powerful boss that appears once your score crosses a threshold.

The game saves your **high score** automatically and provides colorful console graphics, complete with thruster animations and optional sound playback.

---

## ✨ Features

- 🎮 **Keyboard & Xbox Controller Support (via XInput)**
- 💥 **Boss battles** with health bars and multiple attack patterns
- 🔫 **Upgradeable weapons** after defeating a boss
- 🧱 **Dynamic ASCII graphics** with color effects and animations
- 💾 **Automatic high score saving** to `highscore.txt`
- 🔊 **Optional sound playback** using Windows MCI
- 🧠 **Smooth player movement** and responsive shooting mechanics

---

## ⚙️ Installation

### **Requirements**
- Windows OS  
- C++ compiler supporting C++11 or later (e.g., **MSVC**, **MinGW**)  
- `xinput.lib` and `winmm.lib` (included in Windows SDK)

### **Build Instructions**

1. Clone the repository:
   ```bash
   git clone https://github.com/sanskar0780-cpp/Ascii-spaceship-game.git
   cd Ascii-spaceship-game
   ```

2. Compile the code using:
   ```bash
   g++ ConsoleApplication1.cpp -o spaceship.exe -lwinmm -lxinput
   ```

3. Run the game:
   ```bash
   ./spaceship.exe
   ```

---

## 🧭 Usage

When you start the game, an ASCII title screen appears.  
Press any key to begin and pilot your spaceship across the console battlefield.  
Destroy enemies, survive waves, and aim for the highest score possible!

A file named `highscore.txt` is used to save your best performance automatically.

---

## 🎮 Controls

| Input | Action |
|--------|--------|
| **W / S** | Move up / down |
| **Spacebar** | Shoot |
| **Q** | Quit game |
| **Xbox Controller – D-pad / Left Stick** | Move up/down |
| **Xbox Controller – A Button** | Shoot |
| **Xbox Controller – B Button** | Quit game |

---

## ⚔️ Game Mechanics

- **Enemies** spawn periodically and move leftward across the screen.  
- **Bullets** destroy enemies and grant +50 points each.  
- **Boss** appears once you reach certain score thresholds (e.g., 100, 500...).  
- **Boss Health** is displayed on a side panel as a progress bar.  
- Defeating a boss grants weapon upgrades and additional score.  
- **Collision Detection:** Getting hit by an enemy or boss projectile ends the game.

---

## 📦 Dependencies

| Library | Purpose |
|----------|----------|
| `<windows.h>` | Console and system interaction |
| `<xinput.h>` | Xbox controller input |
| `<mmsystem.h>` | Sound and multimedia support |
| `<conio.h>` | Keyboard input handling |
| `<thread>` & `<chrono>` | Frame timing and animations |
| `<fstream>` | High score persistence |

---

## ⚙️ Configuration

You can adjust some gameplay parameters directly in the source code:

| Variable | Description | Default |
|-----------|--------------|----------|
| `enemySpawnRate` | Frames between enemy spawns | `50` |
| `enemySpeed` | Enemy movement speed | `10` |
| `bossSpawnScr` | Score threshold for boss spawn | `100` |
| `bossShootRate` | Frames between boss shots | `60` |
| `bossMoveSpeed` | Boss movement interval | `30` |
| `shootCooldownMax` | Delay between shots | `4` |

---

## 🧰 Troubleshooting

| Issue | Possible Fix |
|--------|---------------|
| **No controller input** | Ensure your Xbox controller is connected and drivers are installed. |
| **Console too small** | Resize your terminal to at least **120x50 characters**. |
| **No sound** | Ensure `winmm.lib` is linked and MCI audio file path is correct. |
| **High score not saving** | Verify the program has permission to write to `highscore.txt`. |

---

## 🚀 Future Enhancements

- [ ] Soundtrack and sound effects integration  
- [ ] Multiple enemy types and projectiles  
- [ ] Save/load system for upgrades  
- [ ] Main menu and settings screen  
- [ ] Linux/Unix compatibility  

---

## 📜 License

This project is licensed under the **MIT License** — free to use, modify, and distribute with attribution.

---

### 👨‍💻 Author
Developed by **Sanskar**  
GitHub: [@sanskar0780-cpp](https://github.com/sanskar0780-cpp)
