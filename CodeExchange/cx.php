<?php
// CodeExchange (cx) v3.0.0 — PHP Implementation

define('IO_BUFFER_SIZE', 16384);

function emit_raw_buffer($output, $buffer) {
    if (strlen($buffer) === 0) return;
    $crc = 0;
    $len = strlen($buffer);
    for ($i = 0; $i < $len; $i++) {
        $crc ^= ord($buffer[$i]);
    }
    fwrite($output, $buffer . "|" . sprintf("%02X", $crc) . "\n");
}

function flush_run(&$buffer, $char, $count, $output, $line_length) {
    if ($count === 0) return;

    $normal_seq = match($char) {
        "\n" => "\\n", "\t" => "\\t", "\r" => "\\r", "\\" => "\\\\", default => $char
    };
    $normal_width = strlen($normal_seq);
    $max_allowed = ($line_length > 4) ? ($line_length - 4) : $line_length;

    while ($count > 0) {
        $chunk = min($count, 65535);
        $raw_cost = $normal_width * $chunk;
        $rle_cost = ($chunk > 255) ? (6 + $normal_width) : (4 + $normal_width);

        if ($line_length > 0 && (strlen($buffer) + $rle_cost > $max_allowed)) {
            if (strlen($buffer) > 0) {
                emit_raw_buffer($output, $buffer);
                $buffer = "";
                continue;
            }
        }

        if ($rle_cost < $raw_cost) {
            $buffer .= ($chunk > 255) ? ("/z" . sprintf("%04X", $chunk)) : ("/x" . sprintf("%02X", $chunk));
            $buffer .= $normal_seq;
            $count -= $chunk;
        } else {
            $process_now = $chunk;
            if ($line_length > 0) {
                $space_left = max(0, $max_allowed - strlen($buffer));
                $max_raw = intdiv($space_left, $normal_width);
                if ($max_raw === 0) {
                    emit_raw_buffer($output, $buffer);
                    $buffer = "";
                    continue;
                }
                $process_now = min($process_now, $max_raw);
            }
            $buffer .= str_repeat($normal_seq, $process_now);
            $count -= $process_now;
        }

        if ($line_length > 0 && strlen($buffer) >= $max_allowed) {
            emit_raw_buffer($output, $buffer);
            $buffer = "";
        }
    }
}

function encode_file($output, $file_path, $line_length) {
    $file = fopen($file_path, "rb");
    if (!$file) return;

    fwrite($output, "===== FILE: $file_path - LL:$line_length ====\n");
    $global_crc = 0;
    $line_buffer = "";
    $current_ch = null;
    $run_count = 0;

    while (!feof($file)) {
        $chunk = fread($file, IO_BUFFER_SIZE);
        $len = strlen($chunk);
        for ($i = 0; $i < $len; $i++) {
            $ch = $chunk[$i];
            $global_crc ^= ord($ch);

            if ($run_count === 0) {
                $current_ch = $ch;
                $run_count = 1;
            } elseif ($ch === $current_ch) {
                $run_count++;
            } else {
                flush_run($line_buffer, $current_ch, $run_count, $output, $line_length);
                $current_ch = $ch;
                $run_count = 1;
            }
        }
    }

    if ($run_count > 0) flush_run($line_buffer, $current_ch, $run_count, $output, $line_length);
    if (strlen($line_buffer) > 0) emit_raw_buffer($output, $line_buffer);

    fwrite($output, "===== ENDFILE: $file_path - FCS:" . sprintf("%02X", $global_crc) . " ====\n");
    fclose($file);
}
