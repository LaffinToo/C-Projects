-- CodeExchange (cx) v3.0.0 — Lua Implementation

local function emit_raw_buffer(output, buffer)
    if #buffer == 0 then return end
    local crc = 0
    for i = 1, #buffer do
        crc = bit32.bxor(crc, string.byte(buffer, i))
    end
    output:write(buffer .. "|" .. string.format("%02X", crc) .. "\n")
end

local function flush_run(buffer, char, count, output, line_length)
    if count == 0 then return buffer end
    
    local escape_map = { ["\n"]="\\n", ["\t"]="\\t", ["\r"]="\\r", ["\\"]="\\\\" }
    local normal_seq = escape_map[char] or char
    local normal_width = #normal_seq
    local max_allowed = line_length > 4 and (line_length - 4) or line_length

    while count > 0 do
        local chunk = math.min(count, 65535)
        local raw_cost = normal_width * chunk
        local rle_cost = chunk > 255 and (6 + normal_width) or (4 + normal_width)

        if line_length > 0 and (#buffer + rle_cost > max_allowed) then
            if #buffer > 0 then
                emit_raw_buffer(output, buffer)
                buffer = ""
            end
        end

        if rle_cost < raw_cost then
            local marker = chunk > 255 and string.format("/z%04X", chunk) or string.format("/x%02X", chunk)
            buffer = buffer .. marker .. normal_seq
            count = count - chunk
        else
            local process_now = chunk
            if line_length > 0 then
                local space_left = math.max(0, max_allowed - #buffer)
                local max_raw = math.floor(space_left / normal_width)
                if max_raw == 0 then
                    emit_raw_buffer(output, buffer)
                    buffer = ""
                else
                    process_now = math.min(process_now, max_raw)
                end
            end
            buffer = buffer .. string.rep(normal_seq, process_now)
            count = count - process_now
        end

        if line_length > 0 and #buffer >= max_allowed then
            emit_raw_buffer(output, buffer)
            buffer = ""
        end
    end
    return buffer
end

function encode_file(output, file_path, line_length)
    local file = io.open(file_path, "rb")
    if not file then return end

    output:write("===== FILE: " .. file_path .. " - LL:" .. line_length .. " ====\n")
    local global_crc = 0
    local line_buffer = ""
    local current_ch = nil
    local run_count = 0

    while true do
        local chunk = file:read(16384)
        if not chunk then break end
        for i = 1, #chunk do
            local ch = string.sub(chunk, i, i)
            global_crc = bit32.bxor(global_crc, string.byte(ch))

            if run_count == 0 then
                current_ch = ch
                run_count = 1
            elseif ch == current_ch then
                run_count = run_count + 1
            else
                line_buffer = flush_run(line_buffer, current_ch, run_count, output, line_length)
                current_ch = ch
                run_count = 1
            end
        end
    end

    if run_count > 0 then line_buffer = flush_run(line_buffer, current_ch, run_count, output, line_length) end
    if #line_buffer > 0 then emit_raw_buffer(output, line_buffer) end

    output:write(string.format("===== ENDFILE: %s - FCS:%02X ====\n", file_path, global_crc))
    file:close()
end
