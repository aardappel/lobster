

gl.window("press the spacebar?", 300, 300)
text_to_speech("Hi, I am the text to speech engine. Please refrain from hitting the spacebar.")
while gl.frame():
    if gl.button("space") == 1:
        text_to_speech("stop hitting the spacebar")
