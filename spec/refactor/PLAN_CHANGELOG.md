# Plan Changelog

This file records small plan updates, status changes, and decisions made during the refactor.

## 2025-11-09 (Evening) - Strategy Migration Attempt & API Discovery
- **StrategyTimeUtils Complete**: Created and committed shared time utility
  - Platform-independent GMT conversion (gmtime_s/gmtime_r/fallback)
  - formatTime() wrapper for strftime
  - timeToUIString() for UI-friendly display
  - Committed to cpp-migration-phase2 (commit 7f7cd48)
  - Successfully replaces legacy safe_timeString

- **Strategy Migration Attempt - Lessons Learned**:
  - Attempted migration of RecordBarsStrategy and TakeOverStrategy
  - Build failures revealed significant API gaps between expected and actual Phase 2
  - **Critical Discovery**: Strategies were written against imagined APIs that don't exist
  
- **API Gaps Identified**:
  - `rates_t` structure lacks: barsTotal, timeArray, openArray, highArray, lowArray, closeArray, volumeArray
  - `StrategyContext` lacks: getBarsTotal(), getTime(), getOpen/High/Low/Close/Volume() accessors
  - `OrderManager` lacks: getInstance(), modifyTradeEasy(), closeAllLongs/Shorts(), totalOpenOrders()
  - `pantheios` lacks: integer() and real() helper functions
  - Type mismatch: StrategyResult.code (StrategyErrorCode vs AsirikuyReturnCode)
  - Missing: addValueToUI() utility function

- **Decision Made**: Reverted all strategy migration work, kept only StrategyTimeUtils
  - Strategy files deleted/restored to original state
  - Clean workspace with only time utility committed
  - Recognized that full API discovery phase needed before migrations can proceed

- **Plan Updates**:
  - Marked RecordBarsStrategy and TakeOverStrategy as **BLOCKED** in TASKS.md
  - Added API Discovery requirements to PLAN_PHASE2_MIGRATION.md
  - Updated blockers section with API discovery requirements
  - Added comprehensive lessons learned notes

- **Recommendations**:
  - Create API_DISCOVERY.md to document actual vs expected APIs
  - Analyze existing C strategies to understand actual data access patterns
  - Validate infrastructure components before assuming convenience methods exist
  - Start next migration with discovery phase, not assumption phase

## 2025-11-09 (Morning)
- Bumped plan documents to version 1.1 and updated `Last Updated` date.
- Marked Phase 1 foundation artifacts implemented: `StrategyContext`, `IStrategy`, `BaseStrategy`, `StrategyResult`.
- Noted blocker: build verification is blocked by missing `premake4` binary and `msbuild` (VS Build Tools) in the current environment.
- Updated `TASKS.md` statuses: foundation tasks completed, next priority is `StrategyFactory` and `Indicators`/`OrderManager` wrappers.
- Added notes to Phase 1 and Phase 2 plans about next actions and sequencing.

---

Older entries remain in document history (git commits).