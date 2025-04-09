# Contributing to OAAX

> This is a work in progress. The content of this document is subject to change as the project evolves. Please check back regularly for updates.   
> Last updated: 2025-04-09 (YYYY-MM-DD)

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
  - [Coding Standards](#coding-standards)
  - [Commit Messages](#commit-messages)
- [License](#license)
- [Code of Conduct](#code-of-conduct)

## How Can I Contribute?

OAAX is a collaborative effort that relies on the support of its community to grow and improve. Here are several ways you can contribute:

### Reporting Bugs

Help us improve OAAX by identifying and reporting bugs across any of our implementations, tools, or examples. Clear and detailed bug reports help maintainers quickly identify and resolve issues.

### Improving Documentation

Good documentation is critical for adoption. You can contribute by enhancing our user guides, API references, and setup instructions—making OAAX easier to use and understand for everyone.

### Creating examples and tutorials

Develop real-world examples and step-by-step tutorials that show how to use OAAX effectively. This could include sample applications, model conversion workflows, or hardware-specific deployment guides.

### Suggesting Enhancements

Have an idea for a new feature or improvement? Propose enhancements that improve the usability, performance, or extensibility of OAAX tools, runtimes, or documentation.

### Contributing to OAAX implementations

Contribute code to OAAX toolchains and runtimes. This could involve:

- Implementing support for a new AI accelerator
- Fixing bugs or addressing edge cases
- Improving the performance and robustness of existing implementations.

Refer to the `README.md` of the specific implementation repository for build and contribution instructions.

### Proposing changes to the standard

OAAX is an evolving standard. If you see opportunities to extend or refine the specification—whether to support new model types, execution paradigms, or hardware features—we welcome proposals and discussions.

## Development Guidelines

### Branching Model

- **Main Branch**: The `main` branch contains the latest stable code.
- **Feature Branches**: Create feature branches from `main` for new features or fixes. Use descriptive names, e.g., `feature/add-new-runtime-support`.
- **Pull Requests**: Submit pull requests (PRs) against the `main` branch. Ensure your PR is based on the latest `main` branch.
- **Review Process**: All PRs must be reviewed by at least one other contributor before merging. The reviewer should ensure that the code adheres to the coding standards and passes all tests.

### Coding Standards

> A formal coding style guide will be published soon. Contributions aligning with consistent, readable, and idiomatic coding practices are welcome.


### Commit Messages

Clear and consistent commit messages help maintain project clarity and assist reviewers during code review.

- Use the imperative mood in commit messages (e.g., "add support for new backend", not "added support").
- Keep the summary line under 72 characters.
- Each commit must be signed off with your name and email address, in compliance with the Developer Certificate of Origin (DCO).


## License

By contributing to OAAX, you agree that your contributions will be licensed under the [Apache License 2.0](LICENSE).

## Code of Conduct

Please note that this project is released with a [Contributor Code of Conduct](CODE_OF_CONDUCT.md). By participating in this project, you agree to abide by its terms.
