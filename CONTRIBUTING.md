# Contributing to OAAX

> This is a work in progress. The content of this document is subject to change as the project evolves. Please check back regularly for updates.

Thank you for your interest in contributing to the Open AI Accelerator eXchange (OAAX) project! OAAX is an open standard that aims to simplify and standardize the deployment of AI models across diverse hardware accelerators. By contributing, you're helping us move closer to our mission of democratizing edge AI.

This document outlines the ways you can contribute and the guidelines to follow when doing so.

## Table of Contents

- [How Can I Contribute?](#how-can-i-contribute)
  - [Reporting Bugs](#reporting-bugs)
  - [Improving Documentation](#improving-documentation)
  - [Creating examples and tutorials](#creating-examples-and-tutorials)
  - [Suggesting Enhancements](#suggesting-enhancements)
  - [Contributing to OAAX implementations](#contributing-to-oaax-implementations)
  - [Proposing changes to the standard](#proposing-changes-to-the-standard)
- [Development Guidelines](#development-guidelines)
  - [Branching Model](#branching-model)
  - [Before Opening a Pull Request](#before-opening-a-pull-request)
  - [Coding Standards](#coding-standards)
  - [Commit Messages and DCO Sign-off](#commit-messages-and-dco-sign-off)
- [License](#license)
- [Code of Conduct](#code-of-conduct)

## How Can I Contribute?

OAAX is a collaborative effort that relies on the support of its community to grow and improve. Here are several ways you can contribute:

### Reporting Bugs

Help us improve OAAX by identifying and reporting bugs across any of our implementations, tools, or examples. Clear and detailed bug reports help maintainers quickly identify and resolve issues.

- **Where**: open an issue in the repository of the affected component (see [the repository map](#contributing-to-oaax-implementations)). The specification, the documentation site and the illustrative example live in this repository. Questions and ideas that are not bugs belong in [Discussions](https://github.com/OAAX-standard/OAAX/discussions).
- **What to include**: the OAAX version and the component version (runtime or toolchain release, or commit), the operating system and CPU architecture, the accelerator, the exact steps to reproduce, what you expected versus what happened, and the relevant logs (toolchain console output and conversion log, or the runtime's error output).
- **Security vulnerabilities**: do not open a public issue; follow [SECURITY.md](SECURITY.md).

### Improving Documentation

Good documentation is critical for adoption. You can contribute by enhancing our user guides, API references, and setup instructions—making OAAX easier to use and understand for everyone.

The documentation site is built from the [`docs/`](docs) folder (the specification is [`docs/Specification.md`](docs/Specification.md)); every "Edit on GitHub" link on the site opens the matching file. Merged changes go live on docs.oaax.org (see [Merging](#branching-model)), and commits need a [DCO sign-off](#commit-messages-and-dco-sign-off), also when made in the web editor.

### Creating examples and tutorials

Develop real-world examples and step-by-step tutorials that show how to use OAAX effectively. This could include sample applications, model conversion workflows, or hardware-specific deployment guides.

### Suggesting Enhancements

Have an idea for a new feature or improvement? Propose enhancements that improve the usability, performance, or extensibility of OAAX tools, runtimes, or documentation.

### Contributing to OAAX implementations

Contribute code to OAAX toolchains and runtimes. This could involve:

- Implementing support for a new AI accelerator
- Fixing bugs or addressing edge cases
- Improving the performance and robustness of existing implementations.

Each repository of the [OAAX-standard organization](https://github.com/OAAX-standard) takes a different kind of contribution:

| Repository | Take it there |
| --- | --- |
| [OAAX](https://github.com/OAAX-standard/OAAX) (this one) | The specification, the documentation site, the runtime interface header, the illustrative example |
| [reference-implementation](https://github.com/OAAX-standard/reference-implementation) | A CPU toolchain and runtime. **Start here** when adding a new accelerator |
| [examples](https://github.com/OAAX-standard/examples) | Examples of using a toolchain and a runtime in an application |
| `<vendor>-acceleration` ([nvidia](https://github.com/OAAX-standard/nvidia-acceleration), [intel](https://github.com/OAAX-standard/intel-acceleration), [hailo](https://github.com/OAAX-standard/hailo-acceleration), [deepx](https://github.com/OAAX-standard/deepx-acceleration), [memryx](https://github.com/OAAX-standard/memryx-acceleration)) | The toolchain and runtime of one accelerator |
| [steering-committee](https://github.com/OAAX-standard/steering-committee) | Governance, meeting notes and the roadmap |

Refer to the `README.md` of the specific implementation repository for build and contribution instructions. To add a new accelerator, fork the reference implementation and start a [Discussion](https://github.com/OAAX-standard/OAAX/discussions) so the maintainers can agree with you on where the result will live.

The runtime API every implementation must follow is defined in [`include/interface.h`](include/interface.h), and the [`example/`](example) folder contains a minimal, runnable toolchain, runtime and host to start from (`bash example/run-example.sh`).

### Proposing changes to the standard

OAAX is an evolving standard. If you see opportunities to extend or refine the specification—whether to support new model types, execution paradigms, or hardware features—we welcome proposals and discussions.

1. **Typos and clarifications**: open a pull request directly.
2. **Anything else**: start a [Discussion](https://github.com/OAAX-standard/OAAX/discussions) (or an issue) describing the problem, the proposed change and its **compatibility impact**: do existing runtimes, toolchains and hosts keep working unchanged?
3. **Decisions** on the interface, the bundle format or the roadmap are taken by the [steering committee](https://github.com/OAAX-standard/steering-committee#how-it-works). To get yours on the agenda, open an issue in that repository with the **TSC agenda** form (it applies the `tsc-agenda` label) saying in one line what decision you want. Decisions are recorded in the meeting notes.
4. Once decided, open the pull request against this repository and link the decision.

## Development Guidelines

### Branching Model

- **Main Branch**: The `main` branch contains the latest stable code.
- **Feature Branches**: Create feature branches from `main` for new features or fixes. Use descriptive names, e.g., `feature/add-new-runtime-support`.
- **Forks**: External contributors work from a fork of the repository; maintainers can push branches directly.
- **Pull Requests**: Submit pull requests (PRs) against the `main` branch (the repository's default branch in the other repositories). Ensure your PR is based on the latest `main` branch. Keep each PR to one change and describe what it does and why.
- **Review Process**: Maintainers review PRs and check that they follow the coding standards and pass all tests. There is no fixed review turnaround; if your PR is waiting, add a comment on it.
- **Merging**: `main` is protected, so nothing is pushed to it directly. Branch protection requires the `DCO` check, and the other CI jobs should pass too; a job that needs repository secrets (for example an artifact upload in a vendor repository) can fail on a PR from a fork without your change being the cause. A maintainer merges the PR. Merging to `main` in this repository publishes the documentation site, so a merged docs change goes live.

### Before Opening a Pull Request

CI runs the same checks, so run the ones that match your change first:

```bash
# Linux with Docker, gcc and cmake (otherwise rely on CI)
bash example/run-example.sh        # runtime interface or example

# anything under docs/ or mkdocs.yml, in a throwaway virtual environment
python3 -m venv /tmp/oaax-docs-venv && . /tmp/oaax-docs-venv/bin/activate   # Debian/Ubuntu: apt install python3-venv
pip install "mkdocs<2" mkdocs-material && mkdocs build --strict
```

### Coding Standards

There is no single OAAX style guide. Follow the style of the code around your change, and run the checks the repository names in its `README.md` or `CONTRIBUTING.md` before you push; where it has a `.pre-commit-config.yaml`, that is `pip install pre-commit && pre-commit run --all-files` (some hooks need Docker). One rule applies everywhere: the public runtime interface stays a pure C API (`extern "C"`), as in [`include/interface.h`](include/interface.h).

### Commit Messages and DCO Sign-off

Clear and consistent commit messages help maintain project clarity and assist reviewers during code review.

- Use the imperative mood in commit messages (e.g., "add support for new backend", not "added support").
- Keep the summary line under 72 characters.
- Each commit must be signed off with your name and email address, in compliance with the [Developer Certificate of Origin](https://developercertificate.org) (DCO). Add `-s` when committing:

  ```bash
  git commit -s -m "Add support for the new backend"
  ```

  This appends a `Signed-off-by: Your Name <you@example.com>` line. The name and email must match the commit's author (`git config user.name` / `user.email`).
- If the DCO check fails on your PR, some commits lack a matching sign-off. Fix them all and update the branch (this also sets each commit's author to your current git identity; `<repository>` is the repository you contribute to and `<base>` the branch your PR targets, `main` unless its guide says otherwise):

  ```bash
  git config user.name "Your Name" && git config user.email "you@example.com"   # as on your GitHub account
  git remote add upstream https://github.com/OAAX-standard/<repository>.git && git fetch upstream   # once, if you work from a fork
  git rebase --exec 'git commit --amend --no-edit --reset-author -s' upstream/<base>   # origin/<base> if you pushed your branch to the repository itself
  git push --force-with-lease origin HEAD
  ```

  Commits made in GitHub's web editor are not signed off: add `Signed-off-by: Your Name <you@example.com>` as the last line of the commit message.


## License

By contributing to OAAX, you agree that your contributions will be licensed under the [Apache License 2.0](LICENSE).

## Code of Conduct

Please note that this project is released with a [Contributor Code of Conduct](CODE_OF_CONDUCT.md). By participating in this project, you agree to abide by its terms.
