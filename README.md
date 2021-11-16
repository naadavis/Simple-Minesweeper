# Simple-Minesweeper

Recreating human style minesweeper play with an automated solver.

Uses neighboring squares' information to avoid brute force state-space style solving.

Relies on a depth first search if initial reasoning fails.

Will make guesses based on state-space generated probabilities.

## Motivations
- Generate an approximnate win percentage ceiling for minesweeper play.
- Asses computational and development complexity of human style minesweeper play.

## Results
- Over 2000 simulated games: 623 wins. 31.15%
  - Given most games will include at least one 50/50 guess, and many will have multiple. Good result.
- Human reasoning is intuitive to use, but not elegent to implement per se.
- Unsurprisingly, recursive state space search is elegant but as unsophisticated as any brute force method.
- However, state space search is necessary to inform guessing accurately.
