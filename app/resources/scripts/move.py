from alabaster_entity import Entity

class MoveableEntity(Entity):
    def __init__(self):
        super().__init__()
        self.t = self.get_transform()
    
    def on_update(self):
        self.t += 1.0


if __name__ == "__main__":
    a = MoveableEntity()
    for i in range(10):
        a.on_update()

