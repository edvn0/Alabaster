from alabaster_ecs import Entity

class Behaviour(Entity):
    def __init__(self, entity):
        super().__init__(entity)
        self.acc = 0

    def on_create(self):
        print("Created!")

    def on_update(self, ts:float):
        self.acc += ts
        if self.acc >= 1:
            t = self.get_transform()
            self.acc = 0

            print(t.position)
            t.position += float(2) * ts

    def on_delete(self):
        print("Deleted!")


