from alabaster_ecs import Entity

class Behaviour(Entity):
	def __init__(self):
		super().__init__()
		
	def on_create(self):
		pass
		
	def on_update(self, ts):
		pass

	def on_delete(self):
		pass
