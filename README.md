# prc24s - Process Tree Management Tool

## Overview

The **prc24s** program is a C-based tool that interacts with process trees, providing functionalities like listing, killing, pausing, and resuming processes based on user input. It is designed to operate using the Linux process tree structure, allowing process management and analysis through various command-line options.

## Features

- **Basic Process Info**: Lists the PID and PPID of a specified process if it belongs to the process tree rooted at a specific root process.

### Process Actions:

- **-dx**: Kills all descendants of the root process.
- **-dt**: Pauses all descendants of the root process.
- **-dc**: Resumes paused descendants.
- **-rp**: Kills a specified process.
- **-nd**: Lists the PIDs of non-direct descendants.
- **-dd**: Lists the PIDs of immediate descendants.
- **-sb**: Lists the sibling PIDs.
- **-bz**: Lists the sibling PIDs that are defunct (zombie).
- **-zd**: Lists all defunct descendants.
- **-od**: Lists orphan descendants.
- **-gc**: Lists the grandchildren.
- **-sz**: Displays if a process is defunct.
- **-so**: Displays if a process is orphaned.
- **-kz**: Kills the parents of all zombie descendants.

## Usage

The program accepts a variety of command-line options to interact with and manage processes in a tree structure. Please refer to the attached PDF for more details about usage.

## Notes

- This program must be implemented on the university's CS Linux server using your official login.
- Be cautious of creating fork bombs; use `$killall -u username` to terminate processes regularly.
- Adequate comments must be included to explain the working of the program.

## License

This project is for academic use only as part of COMP-8567 coursework for Summer 2024.
