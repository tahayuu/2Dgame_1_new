# Copilot Instructions

## General Guidelines
- First general instruction
- Second general instruction

## Game Mechanics
- Implement player death upon enemy side collision (respawn); player should survive when landing on an enemy from above.
- Implement damage to the player when landing on an enemy from above, causing the player to bounce.
- Use the relationship between the player's bottom edge and the enemy's top edge, along with downward velocity, to determine landing collision.
- Ensure stable landing detection without fall motion during walking, as the user prioritizes this aspect.
- Anchor Ojisan to the screen's bottom-left as a fixed UI element, unaffected by camera movement.
- Implement a separate texture mode, activated by the B key, in addition to the stage editing mode (V).

## Font and Dialog Management
- Ensure the dialog file is included in font codepoints and LoadFromFile timing to prevent garbled Japanese text when jumping high.
- Move font-loading code out of the main function and combine it with dialog content for better organization and performance.