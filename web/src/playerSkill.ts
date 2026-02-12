import { lerp } from './utils.js';

let skill = 0.5;

export const PlayerSkill = {
  get(): number { return skill; },

  playerDidSomethingClever(howClever: number, howImportant: number): void {
    if (howClever > skill) {
      skill = lerp(skill, howClever, howImportant);
    }
  },

  playerDidSomethingStupid(howClever: number, howImportant: number): void {
    if (howClever < skill) {
      skill = lerp(skill, howClever, howImportant);
    }
  },

  reset(): void { skill = 0.5; },
} as const;
