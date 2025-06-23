# GOLD ESSENCE: Pragmatic Purity - Encapsulating a common, complex task.
from PIL import Image

class ImageTool(BaseTool):
    @property
    def name(self) -> str: return "image.process"

    async def execute(self, params: dict):
        operation = params.get('operation')
        source = params.get('source_path')
        dest = params.get('dest_path')
        
        img = Image.open(source)

        if operation == 'resize':
            width = params.get('width')
            img = img.resize((width, int(img.height * width / img.width)))
        elif operation == 'watermark':
            watermark_text = params.get('text', '© GAP')
            # ... (logic for adding text overlay) ...
        
        img.save(dest)
        return {'status': 'success', 'path': dest}
