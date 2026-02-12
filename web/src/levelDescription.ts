export class LevelDescription {
  constructor(
    public readonly name: string,
    public readonly levelFile: string,
    public readonly frameIndex: number,
    public readonly musicFile: string,
  ) {}
}
