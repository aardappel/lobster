//
//  Shader.fsh
//  lobster_ios
//
//  Created by Wouter van Oortmerssen on 12/5/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

varying lowp vec4 colorVarying;

void main()
{
    gl_FragColor = colorVarying;
}
