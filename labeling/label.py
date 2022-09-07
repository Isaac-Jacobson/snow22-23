import streamlit as st
import pandas as pd
import numpy as np
import os
from PIL import Image

def initializeIterator():
    #st.write(st.session_state.directory)
    # NOT RECOMMENDED use of os.scandir (os.scandir.close() should be called explicitly or used in *with* statement)
    st.session_state.iterator = os.scandir(st.session_state.directory)
    
    # Initialize the output csv file
    with open(str(st.session_state.directory) + "_ANNOTATIONS.csv", 'w') as f:
        f.write("Flake_Name, Class\n")

    st.session_state.initial = True
    st.session_state.caption = next(st.session_state.iterator).name
    st.session_state.image = Image.open(st.session_state.directory + "\\" + st.session_state.caption)

def restart():
    for key in st.session_state.keys():
        del st.session_state[key]

def updateImageNext():
    #st.session_state.initial = False
        
    '''# Write results to file
    with open(str(st.session_state.directory) + "_ANNOTATIONS.csv", 'a') as f:
        f.write(str(st.session_state.caption) + ", " + str(classes) + "\n")'''

    # Set previmage to current image to allow user to go back 1 if they made a mistake
    # Due to nature of python iterators, going back more than 1 requires signifcant overhead as per https://stackoverflow.com/questions/2777188/making-a-python-iterator-go-backwards
    st.session_state.previmage = st.session_state.image
    st.session_state.prevcaption = st.session_state.caption

    # Go to next image
    try:
        st.session_state.caption = next(st.session_state.iterator).name
        st.session_state.image = Image.open(st.session_state.directory + "\\" + st.session_state.caption)
    except StopIteration:
        st.title("Folder Finished!")
        st.button("Restart:", on_click=restart)

# Initialize to the first image in the target directory
# st.session_state used to preserve state of progress through button presses
# See https://docs.streamlit.io/library/api-reference/session-state for more details
if "initial" not in st.session_state:
    st.session_state.directory = st.text_input("Input Directory", value="E:\\Users\\peter\\Desktop\\2_14_2022__8_SnowCROPPED2", \
                                                placeholder="E:\\Users\\peter\\Desktop\\2_14_2022__8_SnowCROPPED2")

    st.button("Start", on_click=initializeIterator)
        

else:
    # Add "selectbox"s below the image to choose the categories the given image falls within
    classes = ["Aggregate (AG)", "Small Particle (SP)", "Graupel (GR)", "Columnar Crystal (CC)", "Planar Crystal (PC)", "No Visible Flake"]
    selected_class = st.selectbox('Select Class:', [None] + classes)

    # Don't think I want to do riming yet
    #riming = st.selectbox("Select Riming Level:", ["No Riming", "Rimed", "Densely Rimed", "Graupel-Like", "Graupel", "No Visible Flake"])

    # Make two columns to make the buttons go where I want them
    col1, col2 = st.columns(2)

    # Put an image at the top of the page
    st.image(st.session_state.image, caption=st.session_state.caption)

    if selected_class is not None:

        # Make two buttons labelled previous and next to go between images
        if col1.button("Previous"):
            # Show previous image to allow fixing mistakes
            st.session_state.image = st.session_state.previmage
            st.session_state.caption = st.session_state.prevcaption

        if col2.button(label="Next", on_click=updateImageNext):
            st.session_state.initial = False
            
            # Write results to file
            with open(str(st.session_state.directory) + "_ANNOTATIONS.csv", 'a') as f:
                f.write(str(st.session_state.prevcaption) + ", " + str(selected_class) + "\n")

            # Set previmage to current image to allow user to go back 1 if they made a mistake
            # Due to nature of python iterators, going back more than 1 requires signifcant overhead as per https://stackoverflow.com/questions/2777188/making-a-python-iterator-go-backwards
            st.session_state.previmage = st.session_state.image
            st.session_state.prevcaption = st.session_state.caption

            # Go to next image
            #st.session_state.caption = next(st.session_state.iterator).name
            #st.session_state.image = Image.open(st.session_state.directory + "\\" + st.session_state.caption)



















    # if st.session_state.iterator is not None
    #   for file in iterator
    #       st.image(file.image)
    #       selected_class = st.selectbox = 
    #       if selected_class is not None:
    #           render_buttons
    #           if next:
    #               write / append
    #       
