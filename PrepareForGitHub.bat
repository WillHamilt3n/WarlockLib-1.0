@echo off
REM WarlockLib - GitHub Release Preparation Script

echo ========================================
echo WarlockLib - GitHub Release Preparation
echo ========================================
echo.

REM Step 1: Organize project
echo [1/5] Organizing project structure...
if exist CleanOrganize.bat (
    call CleanOrganize.bat
) else (
    echo WARNING: CleanOrganize.bat not found!
    echo Please ensure project structure is organized.
)
echo.

REM Step 2: Rename README
echo [2/5] Preparing README for GitHub...
if exist README_GITHUB.md (
    if exist README.md del README.md
    ren README_GITHUB.md README.md
    echo ? README.md ready
) else (
    echo ? README.md already exists
)
echo.

REM Step 3: Check required files
echo [3/5] Checking required files...
set MISSING=0

if not exist LICENSE (
    echo ? LICENSE missing!
    set MISSING=1
) else (
    echo ? LICENSE
)

if not exist README.md (
    echo ? README.md missing!
    set MISSING=1
) else (
    echo ? README.md
)

if not exist API_REFERENCE.md (
    echo ? API_REFERENCE.md missing!
    set MISSING=1
) else (
    echo ? API_REFERENCE.md
)

if not exist CONTRIBUTING.md (
    echo ? CONTRIBUTING.md missing!
    set MISSING=1
) else (
    echo ? CONTRIBUTING.md
)

if not exist CHANGELOG.md (
    echo ? CHANGELOG.md missing!
    set MISSING=1
) else (
    echo ? CHANGELOG.md
)

if not exist .gitignore (
    echo ? .gitignore missing!
    set MISSING=1
) else (
    echo ? .gitignore
)

if not exist CMakeLists.txt (
    echo ? CMakeLists.txt missing!
    set MISSING=1
) else (
    echo ? CMakeLists.txt
)

echo.

if %MISSING%==1 (
    echo ? Some required files are missing!
    echo Please check the output above.
    goto END
)

REM Step 4: Initialize git
echo [4/5] Checking Git initialization...
if exist .git (
    echo ? Git already initialized
) else (
    echo Initializing Git repository...
    git init
    git branch -M main
    echo ? Git initialized
)
echo.

REM Step 5: Summary
echo [5/5] Preparation Summary
echo ========================================
echo.
echo ? Project structure organized
echo ? README.md ready for GitHub
echo ? All required files present
echo ? Git repository initialized
echo.
echo ========================================
echo.
echo ?? WarlockLib is ready for GitHub!
echo.
echo Next steps:
echo 1. Review GITHUB_RELEASE_CHECKLIST.md
echo 2. Create GitHub repository
echo 3. Run: git add .
echo 4. Run: git commit -m "Initial commit: WarlockLib v1.0.0"
echo 5. Run: git remote add origin YOUR_REPO_URL
echo 6. Run: git push -u origin main
echo.
echo See GITHUB_RELEASE_CHECKLIST.md for detailed instructions.
echo.

:END
pause
