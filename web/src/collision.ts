export interface Collidable {
  getCollideTop(): number;
  getCollideLeft(): number;
  getCollideBottom(): number;
  getCollideRight(): number;
}

export function collides(a: Collidable, b: Collidable): boolean {
  if (a.getCollideRight() < b.getCollideLeft()) return false;
  if (a.getCollideLeft() > b.getCollideRight()) return false;
  if (a.getCollideBottom() < b.getCollideTop()) return false;
  if (a.getCollideTop() > b.getCollideBottom()) return false;
  return true;
}

export function collidesRect(
  a: Collidable, left: number, top: number, right: number, bottom: number,
): boolean {
  if (a.getCollideRight() < left) return false;
  if (a.getCollideLeft() > right) return false;
  if (a.getCollideBottom() < top) return false;
  if (a.getCollideTop() > bottom) return false;
  return true;
}
