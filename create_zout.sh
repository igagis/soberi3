#!/bin/sh

rm -rf zout

mkdir zout

#copy binary
cp out/soberi3 zout

cp out/run_pc.sh zout

#copy music
cp out/mus.mp3 zout



#pack resources
cd out

zip ../zout/res.zip gfx/* sfx/* main.res

cd ..



#pack levels
cd out/lvl

cd action
zip ../../../zout/lvl_action.zip *
cd ..


cd puzzle
zip ../../../zout/lvl_puzzle.zip *
cd ..


cd ../..


