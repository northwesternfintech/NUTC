from PIL import Image
import os

def resize_images(folder_path, new_width):
    for root, dirs, files in os.walk(folder_path):
        for file in files:
            print(file)
            if file.lower().endswith(('.png', '.jpg', '.jpeg')):
                img_path = os.path.join(root, file)
                img = Image.open(img_path)
                
                # Calculate new height while maintaining aspect ratio
                width_percent = (new_width / float(img.size[0]))
                new_height = int((float(img.size[1]) * float(width_percent)))
                
                img = img.resize((new_width, new_height))
                
                img.save(img_path)

if __name__ == '__main__':
    folder_path = "./logos"
    new_width = 400  # The width you want for all images

    resize_images(folder_path, new_width)