---
name: teach
description: Create and continue a stateful Markdown course for a topic that the user wants to learn. Use when the user asks for teaching, lessons, a learning path, or the next lesson in a teaching workspace. Create an ordered glossary first, then create one concise Markdown lesson for each glossary entry in order. Apply Simplified Technical English principles to all course text.
---

# Teach

Teach one topic across multiple sessions. Treat the current directory as the teaching workspace.

## Workspace files

- `MISSION.md`: Record the real-world learning goal. Use [MISSION-FORMAT.md](./MISSION-FORMAT.md).
- `GLOSSARY.md`: Define the ordered course plan and its canonical terms. Use [GLOSSARY-FORMAT.md](./GLOSSARY-FORMAT.md).
- `RESOURCES.md`: Record trusted sources and useful communities. Use [RESOURCES-FORMAT.md](./RESOURCES-FORMAT.md).
- `lessons/*.md`: Store the lessons. Use [LESSON-FORMAT.md](./LESSON-FORMAT.md).
- `learning-records/*.md`: Record demonstrated knowledge. Use [LEARNING-RECORD-FORMAT.md](./LEARNING-RECORD-FORMAT.md).
- `NOTES.md`: Record teaching preferences and short working notes.

Create files and directories only when the workflow needs them. Do not create HTML files, reference lessons, interactive widgets, or reusable web assets.

## Required workflow

1. Read all existing workspace files before you change the course.
2. Establish the mission before you create the glossary. If the mission is unclear, ask why the user wants the topic.
3. Create `GLOSSARY.md` before you create the first lesson.
4. Order glossary entries from prerequisites to advanced concepts. Treat this order as the lesson order.
5. For a new lesson, select the first glossary entry that has no lesson. Do not skip an entry.
6. Create one Markdown lesson for that entry. Do not create a separate reference document.
7. Use the next sequential lesson number and the glossary term slug: `0001-term-name.md`, `0002-next-term.md`.
8. Add navigation links as specified in [LESSON-FORMAT.md](./LESSON-FORMAT.md).
9. If a previous lesson exists, update its `Next` link to point to the new lesson.
10. Review the lesson with the Simple English check in this file before you finish.

If the user asks for a later topic, explain that the course follows the glossary order. Offer to reorder the glossary when that change still preserves prerequisites.

## Teaching decisions

Tie each lesson to the mission. Give the learner one useful result from one tightly scoped concept.

Use learning records to estimate the learner's current level. Do not record coverage as learning. Record knowledge only when the user demonstrates it or states relevant prior knowledge.

Use retrieval practice, spacing, and interleaving when they help retention. Keep exercises in Markdown. Ask the user to answer in the conversation or perform a real-world task. Do not add scripts, forms, quizzes with hidden answers, or browser interaction.

Use trusted sources. Prefer primary sources, recognized experts, and peer-reviewed work. Browse when knowledge can change, accuracy is important, or the available resources do not support the lesson. Add useful sources to `RESOURCES.md`. Cite the primary source inside the lesson.

## Simple English rules

Apply these rules to the glossary, lessons, records, notes, and workspace instructions:

- Use pragmatic Simplified Technical English. Keep necessary domain terms.
- Separate procedures from descriptions.
- Use no more than 20 words in a procedural sentence.
- Use no more than 25 words in a descriptive sentence.
- Give one instruction per sentence.
- Put a condition before its instruction: "If the command fails, read the log."
- Use the imperative for instructions. Use simple tenses and active voice elsewhere.
- Use one term for one concept. Follow the canonical terms in `GLOSSARY.md`.
- Use short noun groups. Break noun groups longer than three words with a preposition.
- Do not use contractions, semicolons, phrasal verbs, decorative language, or unnecessary hedges.
- Do not use `should`, `would`, `may`, `might`, or `could`. Use `must`, `can`, or a direct statement.
- Preserve code, commands, identifiers, paths, quotations, product names, and API names exactly.

Before you finish, perform this check:

1. Count the words in the three longest sentences. Split sentences that exceed their applicable limit.
2. Search for contractions, semicolons, perfect tenses, banned modals, and unnecessary `-ing` clauses.
3. Make sure that each condition precedes its instruction.
4. Make sure that one canonical term names each concept throughout the lesson.
5. Make sure that descriptions explain and procedures instruct.

## Course maintenance

The glossary controls lesson order. If the glossary order changes after lessons exist, rename affected lesson files and repair all navigation links in one change.

Do not change the mission without user confirmation. When the mission changes, update `MISSION.md` and add a learning record.

Revise a glossary definition when understanding improves. Keep its canonical term stable unless the old term is wrong or ambiguous.
