# the-dark-dungeon-world-gen-test-
a test program to test the robustness of my game "the dark dungeon"'s world generation code

if you run the program, just press "R" to regenerate the world to see it generates a new one. 

#procedural generation method

1. First generate random rectangles that are apart at least 1 tile. (give each rectangle their unique region id)
2. Carve passengers by flood-filling randomly. (give each path their unique region id)
3. For each rectangle, find potential connectors. Connect them and unify both regions (set the path's region id to the rectangle's)
4. Check the same rectangle again for connectors. (maybe a different region is also nearby).
5. Move on to another rectangle and repeat from 3. If all rectangles have been processed, move onto 6.
6. Find tile that are deadend (only one side is connected). Then recursively fill up the path with rock(wall) until hit a tile that's 
   connected by more than 1 tile. 
7. Repeat 6 for each deadend, then you are done. 

#screenshots:
screenshot 1: (12/28/2016)
![image](https://cloud.githubusercontent.com/assets/16845654/21541818/605e2368-cd6e-11e6-9712-354580963dad.png)

screenshot 2: (12/30/2016)
![image](https://cloud.githubusercontent.com/assets/16845654/21572926/d393cf1a-ce93-11e6-9438-23d5a79acfec.png)
