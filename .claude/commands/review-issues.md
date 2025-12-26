# Review Static Analyzer Issues

Review critical static analyzer issues and record decisions.

## Prerequisites

Run these commands first if not already done:
```bash
make analyze
make analyze-export FMT=sqlite CRITICAL=1
```

## Workflow

1. **Load issues and decisions databases:**
   - `codechecker-report/issues.db` - Fresh analyzer results
   - `codechecker-report/decisions.db` - Persisted decisions (may not exist yet)

2. **Query for undecided issues:**
   ```sql
   -- Issues without decisions
   SELECT i.id, i.severity, i.file_path, i.line, i.checker_name, i.message, i.report_hash
   FROM issues i
   LEFT JOIN decisions d ON i.report_hash = d.report_hash
   WHERE d.report_hash IS NULL
   ORDER BY
     CASE i.severity WHEN 'HIGH' THEN 1 WHEN 'MEDIUM' THEN 2 ELSE 3 END,
     i.file_path, i.line
   ```

3. **For each undecided issue, determine the appropriate action:**
   - **Fix it**: If it's a real bug, implement the fix
   - **false_positive**: Analyzer is wrong; code is correct
   - **wontfix**: Real issue but too risky/costly to fix (e.g., legacy code)
   - **deferred**: Real issue to fix later

4. **Record decisions in decisions.db:**
   ```sql
   INSERT OR REPLACE INTO decisions
   (report_hash, status, rationale, decided_at, file_path, line, checker_name, message)
   VALUES (?, ?, ?, datetime('now'), ?, ?, ?, ?);
   ```

## Decisions Database Schema

```sql
CREATE TABLE IF NOT EXISTS decisions (
    report_hash TEXT PRIMARY KEY,
    status TEXT NOT NULL CHECK(status IN ('false_positive', 'wontfix', 'deferred')),
    rationale TEXT NOT NULL,
    decided_at TEXT NOT NULL,
    file_path TEXT NOT NULL,
    line INTEGER NOT NULL,
    checker_name TEXT NOT NULL,
    message TEXT NOT NULL
);
```

## Example Session

1. Read both databases to understand current state
2. For each HIGH severity undecided issue:
   - Read the relevant source code
   - Analyze whether it's a real bug or false positive
   - Either fix the code OR record a decision with rationale
3. Summarize actions taken

## Notes

- `report_hash` is CodeChecker's stable identifier for an issue
- Decisions persist across `make clean-all` and re-analysis
- Use `decisions prune` logic if needed: remove decisions where report_hash no longer exists in issues.db
