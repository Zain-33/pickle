# Contributing to Pickle 🥒

Thanks for wanting to contribute! Pickle is a small open source robot project and all contributions are welcome.

## How to contribute

### Reporting bugs
- Open an issue on GitHub
- Describe what happened vs what you expected
- Include your hardware setup (motor type, battery, etc.)

### Suggesting features
- Open an issue with the label `enhancement`
- Describe the feature and why it would be useful
- If it involves new hardware, mention the component

### Submitting code changes
1. Fork the repository
2. Create a new branch: `git checkout -b feature/your-feature-name`
3. Make your changes to `pickle/pickle.ino`
4. Test on real hardware before submitting
5. Submit a pull request with a clear description of what changed and why

## Code style guidelines
- Keep comments clear and in English
- Use `#define` for all pin numbers at the top of the file
- Group related pins and constants together with section comments
- Test all three modes (Sleep, Wiggle, Autonomous) before submitting

## Hardware contributions
If you have tested Pickle with different hardware (different motors, sensors, chassis), feel free to open an issue sharing your experience — it helps others building their own Pickle!

## Questions?
Open a GitHub issue with the label `question` and we'll help out.
