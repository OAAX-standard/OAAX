# ❓ OAAX Frequently Asked Questions (FAQ)

Welcome to the OAAX FAQ. Below are answers to common questions about the OAAX standard, its goals, usage, and community.

---

### 🧠 General

**What is OAAX?**
OAAX (Open AI Accelerator eXchange) is an open standard for deploying AI models across heterogeneous hardware (aka. XPUs) using a unified conversion and runtime interface. It enables portability, reproducibility, and modularity for AI model execution at the edge and beyond.

**Why was OAAX created?**
AI hardware is fragmented — each vendor provides custom SDKs, runtimes, and conversion tools. OAAX addresses this fragmentation by standardizing the toolchain and runtime interface, making it easier to adopt new hardware or switch vendors without rewriting code.

**Who is behind OAAX?**
OAAX is a project under the LF AI & Data Foundation. It is developed and maintained by a community of AI hardware vendors, software developers, and platform builders. The initial proposal was led by [Network Optix](https://networkoptix.com).

**What does OAAX stand for?**
**O**pen **A**I **A**ccelerator e**X**change.

---

### 🔧 Usage

**How do I use OAAX in my project?**

1. Convert your ONNX model using the OAAX Toolchain Docker image.
2. Load the resulting OAAX bundle using the OAAX Runtime API.
3. Send inputs and receive outputs through a standardized C API.

See the **Getting Started** guide for full details.

**What model formats are supported?**
Currently, OAAX supports ONNX format as input for model conversion. The output is a binary archive specific to the selected AI hardware.

**Is OAAX only for inference?**
Yes. OAAX currently targets **inference workloads only**. Training workloads are out of scope.

---

### 💡 Technical Details

**How does the runtime interface work?**
OAAX defines a minimal C API with functions such as `runtime_initialization`, `runtime_model_loading`, `send_input`, and `receive_output`. Each hardware vendor implements this API to allow their device to run OAAX-converted models.

**Is the toolchain containerized?**
Yes. The conversion toolchain is packaged as a Docker image to isolate dependencies and ensure reproducibility. This also abstracts away vendor-specific conversion scripts.

**Can I run multiple OAAX runtimes together?**
Yes. Because the runtime interface is standardized and stateless, you can instantiate multiple runtimes — each targeting a different accelerator — in a single application.

**What platforms does OAAX support?**

* **Host OS**: Linux (Ubuntu), Windows (limited)
* **Accelerators**: CPU, Intel, NVIDIA, Hailo, DEEPX, and more in progress.
* **Languages**: C/C++ (runtime), Python bindings coming soon

**Does OAAX support dynamic batching or streaming?**
Support for batching is vendor-specific. The standard itself allows for single or batched inputs; runtime implementations may offer optimizations.

---

### 📦 Contributions

**How do I add support for a new accelerator?**
Vendors can contribute:

* A **toolchain Docker image** that consumes ONNX and produces a valid bundle that can be loaded by the OAAX runtime.
* A **runtime implementation** of the OAAX C API
  See the **Contributor Guide** for instructions.

**Can I propose changes to the OAAX spec?**
Yes. OAAX is an open standard and encourages discussion via GitHub issues, pull requests, and mailing lists. Major changes go through the steering committee for review.

**Where is the source code?**
All repositories are on the [OAAX GitHub organization](https://github.com/OAAX-standard).

---

### 🧪 Compatibility

**Is OAAX compatible with OpenVINO, TensorRT, or other toolkits?**
Yes, OAAX can act as a wrapper around vendor toolkits. For example, the Intel OAAX runtime internally calls OpenVINO. The goal is to **standardize the outer interface** while allowing flexibility inside.

**Does OAAX guarantee performance parity with native SDKs?**
OAAX introduces a thin wrapper layer. While it may add negligible overhead, the performance largely matches native execution — assuming the vendor has optimized their runtime properly.

---

### 🔐 IP and Deployment

**Can I keep my model private when using OAAX?**
Yes. OAAX as a standard allows keeping models private. It's up to the vendor's implementation to handle model compilation (by the toolchain) and loading (by the runtime) securely.

**Is the runtime sandboxed?**
No, the runtime runs natively. However, you can deploy it inside containers, isolated services, or with limited permissions.

---

### 📣 Community

**How do I get help?**

* GitHub Discussions: [OAAX-standard/OAAX](https://github.com/OAAX-standard/OAAX/discussions)
* Slack: Coming soon

**How often does the standard change?**
OAAX aims for **stable, versioned releases**. Breaking changes are minimized to maintain compatibility.

**How can I join the steering group?**
Active contributors or organizations using OAAX in production may request to join the steering committee. See [Governance](../community/governance.md) for details.

---

### 🧾 Licensing & Legal

**What license is OAAX released under?**
OAAX is licensed under the [Apache 2.0 License](https://www.apache.org/licenses/LICENSE-2.0), encouraging open contribution and commercial use.

---

Have a question that isn't answered here?
💬 [Open a GitHub Issue](https://github.com/OAAX-standard/OAAX/issues/new/choose) to help us improve this FAQ.
