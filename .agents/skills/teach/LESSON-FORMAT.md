# Lesson format

Store each lesson in `lessons/NNNN-term-slug.md`. The four-digit number must match the glossary position.

Examples:

- Glossary term 1, "Variables": `lessons/0001-variables.md`
- Glossary term 2, "Data types": `lessons/0002-data-types.md`

Do not infer the next number from the file count. Read the glossary position and inspect existing filenames.

## Template

```md
# {Canonical glossary term}

{One sentence that connects this lesson to the mission.}

## What you will learn

{One observable result for this lesson.}

## Explanation

{A short explanation that uses canonical glossary terms.}

## Example

{One concrete example.}

## Practice

{A short prompt for an answer in the conversation or a real-world task.}

## Source

- [{Primary source title}]({URL}) — {Why this source supports the lesson.}

## Continue

Previous: {`None` or `[Lesson title](./NNNN-slug.md)`}

Next: {`Not created yet` or `[Lesson title](./NNNN-slug.md)`}

Ask your teacher about any part that is not clear.
```

## Navigation procedure

When you create the first lesson, set `Previous` to `None`. Set `Next` to `Not created yet`.

When you create a later lesson:

1. Link its `Previous` field to the preceding lesson.
2. Set its `Next` field to `Not created yet`.
3. Replace `Next: Not created yet` in the preceding lesson with a link to the new lesson.

Use relative links because all lessons are in the same directory. Do not add a next link for a lesson that does not exist.

## Content rules

- Teach only the glossary entry named by the filename.
- Keep the lesson short enough to complete in one sitting.
- Give one tangible result.
- Use plain Markdown only.
- Do not add HTML, embedded scripts, interactive controls, or a separate reference file.
- Use one primary source when possible.
- Put optional sources in `RESOURCES.md` instead of making a long lesson bibliography.
- Apply the Simple English rules in [SKILL.md](./SKILL.md).
