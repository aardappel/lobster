// standard color constants

value color: [ red:float, green:float, blue:float, alpha:float ]

color_black :== [ 0, 0, 0, 1 ]:color
color_white :== [ 1, 1, 1, 1 ]:color

color_red   :== [ 1, 0, 0, 1 ]:color
color_green :== [ 0, 1, 0, 1 ]:color
color_blue  :== [ 0, 0, 1, 1 ]:color

color_yellow :== [ 1, 1, 0, 1 ]:color
color_cyan   :== [ 0, 1, 1, 1 ]:color
color_pink   :== [ 1, 0, 1, 1 ]:color

color_grey          :== [ 0.5,  0.5,  0.5,  1 ]:color
color_dark_grey     :== [ 0.25, 0.25, 0.25, 1 ]:color
color_light_grey    :== [ 0.75, 0.75, 0.75, 1 ]:color
color_lightest_grey :== [ 0.9,  0.9,  0.9,  1 ]:color
color_darkest_grey  :== [ 0.1,  0.1,  0.1,  1 ]:color

color_dark_red   :== [ 0.5,   0,   0, 1 ]:color
color_dark_green :== [   0, 0.5,   0, 1 ]:color
color_dark_blue  :== [   0,   0, 0.5, 1 ]:color

color_light_red   :== [ 1,   0.5, 0.5, 1 ]:color
color_light_green :== [ 0.5, 1,   0.5, 1 ]:color
color_light_blue  :== [ 0.5, 0.5, 1,   1 ]:color

color_olive  :== [ 0.5, 0.5, 0,   1 ]:color
color_teal   :== [ 0,   0.5, 0.5, 1 ]:color
color_purple :== [ 0.5, 0,   0.5, 1 ]:color

color_orange      :== [ 1,   0.5, 0,   1 ]:color
color_chartreuse  :== [ 0.5, 1,   0,   1 ]:color
color_springgreen :== [ 0,   1,   0.5, 1 ]:color
color_dodgerblue  :== [ 0,   0.5, 1,   1 ]:color
color_pink_violet :== [ 1,   0,   0.5, 1 ]:color
color_blue_violet :== [ 0.5, 0,   1,   1 ]:color
