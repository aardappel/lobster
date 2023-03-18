

gl_window("press the spacebar?", 300, 300)
text_to_speech("Hi, I am the text to speech engine. Please refrain from hitting the spacebar.")
while gl_frame():
    if gl_button("space") == 1:
        text_to_speech("stop hitting the spacebar")
