# Testing Network Connection

## How to Test on One PC (Localhost)

### Terminal 1 - Host
```bash
cd /home/lukas/skola/UPR/projekt
./build/projekt

# In the game menu:
1. Click "Multiplayer"
2. Click "Host Game"
3. Wait... (it will block until client connects)
```

### Terminal 2 - Client
```bash
cd /home/lukas/skola/UPR/projekt
./build/projekt

# In the game menu:
1. Click "Multiplayer"
2. Click "Join Game"
3. IP should already be 127.0.0.1 (localhost)
4. Click "Connect"
```

## What Should Happen

**Host terminal:**
```
HOST: Listening on port 7777...
HOST: Waiting for player to connect...
HOST: Player connected!
SUCCESS: Player connected!
Connection established! (Game not implemented yet)
Press any key to return to menu...
```

**Client terminal:**
```
CLIENT: Connecting to 127.0.0.1:7777...
CLIENT: Connected successfully!
SUCCESS: Connected to server!
Connection established! (Game not implemented yet)
Press any key to return to menu...
```

**Both screens will show:**
```
CONNECTION SUCCESSFUL!
Multiplayer game not yet implemented
Press SPACE to return to menu
```

## What This Proves

✅ **SDL2_net is working**
✅ **TCP connection established**
✅ **Host can accept connections**
✅ **Client can connect**
✅ **Menu integration works**

## Next Steps

Now you can implement:
- Sending/receiving messages (Phase 3)
- Game state synchronization (Phase 4)
- Actual multiplayer gameplay (Phases 5-7)

The hardest part (connection setup) is done!
